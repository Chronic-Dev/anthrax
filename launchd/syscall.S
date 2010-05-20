.globl _open
.globl _close
.globl _dup2
.globl _write
.globl _vfork
.globl _wait4
.globl _stat
.globl _mount
.globl _execve
.globl _reboot
.globl _mach_timebase_info
.globl _mach_wait_until
.globl _mach_absolute_time

_dup2:
  mov r12, #0x5A
  swi 128
  bx lr

_write:
  mov r12, #0x4
  swi 128
  bx lr

_vfork:
  eor r0, r0, r0
  mov r12, #0x42
  swi 128
  cmp r1, #0x0
  beq vfork_parent
  mov r0, #0x0
vfork_parent:
  bx lr

_wait4:
  mov r12, #0x7
  swi 128
  bx lr

_mach_timebase_info:
  mvn r12, #0x58
  swi 128
  bx lr

_mach_absolute_time:
  mvn r12, #0x2
  swi 128
  bx lr

_mach_wait_until:
  mvn r12, #0x59
  swi 128
  bx lr

_mlock:
  mov r12, #0xCB
  swi 128
  bx lr

_stat:
  mov r12, #0xBC
  swi 128
  bx lr

_swtch_pri:
  mvn r12, #0x3a
  swi 128
  bx lr

_mount:
  mov r12, #0xA7
  swi 128
  bx lr

_chroot:
  mov r12, #0x3D
  swi 128
  bx lr

_chmod:
  mov r12, #0xF
  swi 128
  bx lr

_execve:
  mov r12, #0x3B
  swi 128
  bx lr

_reboot:
  mov r12, #0x37
  swi 128
  bx lr
  
_close:
  mov r12, #0x6
  swi 128
  bx lr
  
_open:
  mov r12, #0x5
  swi 128
  bx lr
