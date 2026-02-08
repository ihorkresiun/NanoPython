#ifndef __INC_VM_CONFIG_H__
#define __INC_VM_CONFIG_H__

#define VM_DEBUG                (0) // Set to 1 to enable basic debug output, 2 for more verbose debug output

#define VM_STACK_SIZE           (1024)
#define VM_CALL_STACK_SIZE      (64)

#define VM_USE_GC               (1)
#define VM_GC_THRESHOLD         (1024 * 8) // 8 KB

#endif // __INC_VM_CONFIG_H__
