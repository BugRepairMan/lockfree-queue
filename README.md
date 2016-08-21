# lockfree-queue

This is a lockfree queue supporting multi-producer & multi-consumer, which is implemented with C++11 atomic CAS API.
Compared with implementing lockfree via inline asm manually, this C++11 implementation provides portability.

However, you should ensure that your compiler is able to generate the CAS instruction, which requires "-mcx16" flag for clang and gcc.
Actually, the generated CAS is double-width-CAS (DWCAS) used to address the ABA problem in this code. 
The DWCAS is supported by most x86-64 architecture, i.e., CMPXCHG16B.
