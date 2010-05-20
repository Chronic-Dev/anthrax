#include <CoreGraphics/CoreGraphics.h>

CGContextRef framebuffer_open();
CGLayerRef create_image_layer(CGContextRef screen, const char *path);
void framebuffer_close(CGContextRef fb_context);