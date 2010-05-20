#include <IOKit/IOKitLib.h>
#include <CoreFoundation/CoreFoundation.h>
#include <mach/mach.h>
#include <CoreGraphics/CoreGraphics.h>

typedef int kern_return_t;
typedef void *CoreSurfaceBufferRef;
typedef void *IOMobileFramebufferRef;

int CoreSurfaceBufferLock(CoreSurfaceBufferRef surface, unsigned int lockType);
int CoreSurfaceBufferUnlock(CoreSurfaceBufferRef surface);

// globals
int lcdHeight=0,lcdWidth=0;
CoreSurfaceBufferRef fb_surface = NULL;
io_service_t lcdService;

CGContextRef framebuffer_open() {
	CFMutableDictionaryRef queryDict = IOServiceMatching("IOMobileFramebuffer");
	io_iterator_t iterator;
	io_service_t lcd_service;
	
	IOServiceGetMatchingServices(kIOMasterPortDefault, queryDict, &iterator);
	while(lcd_service = IOIteratorNext(iterator)) {
		io_name_t name;
		IORegistryEntryGetName(lcd_service, name);
		if(strstr((char *)name, "LCD") != NULL) {
			break;
		} else {
			IOObjectRelease(lcd_service);
		}
	}
	IOObjectRelease(iterator);
	
	if(!lcd_service) {
		fprintf(stderr, "error: Cannot find LCD service\n");
		fflush(stderr);
		IOObjectRelease(lcd_service);
		
		return NULL;
	}
	
	IOMobileFramebufferRef frameBuffer;
	
	kern_return_t result;
	result = IOMobileFramebufferOpen(lcd_service, mach_task_self(), 0, &frameBuffer);
	if(result != 0) {
		IOObjectRelease(lcd_service);
		return NULL;
	}
	
	result = IOMobileFramebufferGetLayerDefaultSurface(frameBuffer, 0, &fb_surface);
	if(result != 0) {
		IOObjectRelease(lcd_service);
		return NULL;
	}
	
	if(CoreSurfaceBufferLock(fb_surface, 3) != 0) {
		IOObjectRelease(lcd_service);
		return NULL;
	}
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
	void *bufferAddress = (void *)CoreSurfaceBufferGetBaseAddress(fb_surface);
	fflush(stdout);
	
	unsigned int height = CoreSurfaceBufferGetHeight(fb_surface);
	unsigned int width = CoreSurfaceBufferGetWidth(fb_surface);
	unsigned int bytes_per_row = CoreSurfaceBufferGetBytesPerRow(fb_surface);
	
	lcdHeight = height;
	lcdWidth = width;
	
	CGContextRef context = CGBitmapContextCreate(bufferAddress, width, height, 8, bytes_per_row, colorSpace, kCGImageAlphaNoneSkipFirst);
	CGColorSpaceRelease(colorSpace);
	
	if(context == NULL) {
		fprintf(stderr, "error: Could not create CGContext\n");
		fflush(stderr);
		
		IOObjectRelease(lcd_service);
		return NULL;
	} else {
		lcdService = lcd_service;
		return context;
	}
	
	return NULL;
}

CGLayerRef create_image_layer(CGContextRef screen, const char *path) {
	CFStringRef imagePath = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
	if(imagePath == NULL) {
		return NULL;
	}
	
	CFURLRef imageURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, imagePath, kCFURLPOSIXPathStyle, 0);
	if(imageURL == NULL) {
		CFRelease(imagePath);
		return NULL;
	}
	
	CGDataProviderRef dataProvider = CGDataProviderCreateWithURL(imageURL);
	if(dataProvider == NULL) {
		CFRelease(imagePath);
		CFRelease(imageURL);
		return NULL;
	}
	
	CGImageRef image = CGImageCreateWithPNGDataProvider(dataProvider, NULL, 1, kCGRenderingIntentDefault);
	if(image == NULL) {
		CFRelease(imagePath);
		CFRelease(imageURL);
		CGDataProviderRelease(dataProvider);
		return NULL;
	}

	size_t width = CGImageGetWidth(image);
	size_t height = CGImageGetHeight(image);
	
	CGSize img_size;
	img_size.width = width;
	img_size.height = height;
	
	CGLayerRef imgLayer = CGLayerCreateWithContext(screen, img_size, NULL);
	if(imgLayer == NULL) {
		CGImageRelease(image);
		CGDataProviderRelease(dataProvider);
		CFRelease(imageURL);
		CFRelease(imagePath);
		return NULL;
	}
	
	CGContextRef layerContext = CGLayerGetContext(imgLayer);
	CGContextDrawImage(layerContext, CGRectMake(0, 0, 320, 480), image);
	
	CFRelease(imagePath);
	CFRelease(imageURL);
	CGDataProviderRelease(dataProvider);
	CGImageRelease(image);
	
	return imgLayer;
}

void framebuffer_close(CGContextRef fb_context) {
	if(!fb_context) return;
	CoreSurfaceBufferUnlock(fb_surface);
	CGContextRelease(fb_context);
}
