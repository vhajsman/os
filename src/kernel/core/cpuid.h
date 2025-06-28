#ifndef __CPUID_H
#define __CPUID_H

#include "types.h"

// ======================================
// VENDOR STRINGS
// ======================================

#define CPUID_VENDOR_AMD           "AuthenticAMD"
#define CPUID_VENDOR_AMD_OLD       "AMDisbetter!"
#define CPUID_VENDOR_INTEL         "GenuineIntel"
#define CPUID_VENDOR_VIA           "VIA VIA VIA "
#define CPUID_VENDOR_TRANSMETA     "GenuineTMx86"
#define CPUID_VENDOR_TRANSMETA_OLD "TransmetaCPU"
#define CPUID_VENDOR_CYRIX         "CyrixInstead"
#define CPUID_VENDOR_CENTAUR       "CentaurHauls"
#define CPUID_VENDOR_NEXGEN        "NexGenDriven"
#define CPUID_VENDOR_UMC           "UMC UMC UMC "
#define CPUID_VENDOR_SIS           "SiS SiS SiS "
#define CPUID_VENDOR_NSC           "Geode by NSC"
#define CPUID_VENDOR_RISE          "RiseRiseRise"
#define CPUID_VENDOR_VORTEX        "Vortex86 SoC"
#define CPUID_VENDOR_AO486         "MiSTer AO486"
#define CPUID_VENDOR_AO486_OLD     "GenuineAO486"
#define CPUID_VENDOR_ZHAOXIN       "  Shanghai  "
#define CPUID_VENDOR_HYGON         "HygonGenuine"
#define CPUID_VENDOR_ELBRUS        "E2K MACHINE "

#define CPUID_VENDOR_QEMU          "TCGTCGTCGTCG"
#define CPUID_VENDOR_KVM           " KVMKVMKVM  "
#define CPUID_VENDOR_VMWARE        "VMwareVMware"
#define CPUID_VENDOR_VIRTUALBOX    "VBoxVBoxVBox"
#define CPUID_VENDOR_XEN           "XenVMMXenVMM"
#define CPUID_VENDOR_HYPERV        "Microsoft Hv"
#define CPUID_VENDOR_PARALLELS     " prl hyperv "
#define CPUID_VENDOR_PARALLELS_ALT " lrpepyh vr "
#define CPUID_VENDOR_BHYVE         "bhyve bhyve "
#define CPUID_VENDOR_QNX           " QNXQVMBSQG "

// ======================================
// FEATURES
// ======================================

#define CPU_FEAT_SSE3         (ecx & (1 << 0))
#define CPU_FEAT_PCLMULQDQ    (ecx & (1 << 1))
#define CPU_FEAT_DTES64       (ecx & (1 << 2))
#define CPU_FEAT_MONITOR      (ecx & (1 << 3))
#define CPU_FEAT_DS_CPL       (ecx & (1 << 4))
#define CPU_FEAT_VMX          (ecx & (1 << 5))
#define CPU_FEAT_SMX          (ecx & (1 << 6))
#define CPU_FEAT_EST          (ecx & (1 << 7))
#define CPU_FEAT_TM2          (ecx & (1 << 8))
#define CPU_FEAT_SSSE3        (ecx & (1 << 9))
#define CPU_FEAT_CID          (ecx & (1 << 10))
#define CPU_FEAT_FMA          (ecx & (1 << 12))
#define CPU_FEAT_CX16         (ecx & (1 << 13))
#define CPU_FEAT_ETPRD        (ecx & (1 << 14))
#define CPU_FEAT_PDCM         (ecx & (1 << 15))
#define CPU_FEAT_PCID         (ecx & (1 << 17))
#define CPU_FEAT_DCA          (ecx & (1 << 18))
#define CPU_FEAT_SSE4_1       (ecx & (1 << 19))
#define CPU_FEAT_SSE4_2       (ecx & (1 << 20))
#define CPU_FEAT_X2APIC       (ecx & (1 << 21))
#define CPU_FEAT_MOVBE        (ecx & (1 << 22))
#define CPU_FEAT_POPCNT       (ecx & (1 << 23))
#define CPU_FEAT_AES          (ecx & (1 << 25))
#define CPU_FEAT_XSAVE        (ecx & (1 << 26))
#define CPU_FEAT_OSXSAVE      (ecx & (1 << 27))
#define CPU_FEAT_AVX          (ecx & (1 << 28))
#define CPU_FEAT_F16C         (ecx & (1 << 29))
#define CPU_FEAT_RDRAND       (ecx & (1 << 30))

#define CPU_FEAT_FPU          (edx & (1 << 0))
#define CPU_FEAT_VME          (edx & (1 << 1))
#define CPU_FEAT_DE           (edx & (1 << 2))
#define CPU_FEAT_PSE          (edx & (1 << 3))
#define CPU_FEAT_TSC          (edx & (1 << 4))
#define CPU_FEAT_MSR          (edx & (1 << 5))
#define CPU_FEAT_PAE          (edx & (1 << 6))
#define CPU_FEAT_MCE          (edx & (1 << 7))
#define CPU_FEAT_CX8          (edx & (1 << 8))
#define CPU_FEAT_APIC         (edx & (1 << 9))
#define CPU_FEAT_SEP          (edx & (1 << 11))
#define CPU_FEAT_MTRR         (edx & (1 << 12))
#define CPU_FEAT_PGE          (edx & (1 << 13))
#define CPU_FEAT_MCA          (edx & (1 << 14))
#define CPU_FEAT_CMOV         (edx & (1 << 15))
#define CPU_FEAT_PAT          (edx & (1 << 16))
#define CPU_FEAT_PSE36        (edx & (1 << 17))
#define CPU_FEAT_CLFLUSH      (edx & (1 << 19))
#define CPU_FEAT_MMX          (edx & (1 << 23))
#define CPU_FEAT_FXSR         (edx & (1 << 24))
#define CPU_FEAT_SSE          (edx & (1 << 25))
#define CPU_FEAT_SSE2         (edx & (1 << 26))

#define CPU_FEAT_FSGSBASE     (ebx & (1 << 0))
#define CPU_FEAT_BMI1         (ebx & (1 << 3))
#define CPU_FEAT_HLE          (ebx & (1 << 4))
#define CPU_FEAT_AVX2         (ebx & (1 << 5))
#define CPU_FEAT_SMEP         (ebx & (1 << 7))
#define CPU_FEAT_BMI2         (ebx & (1 << 8))
#define CPU_FEAT_ERMS         (ebx & (1 << 9))
#define CPU_FEAT_INVPCID      (ebx & (1 << 10))
#define CPU_FEAT_RTM          (ebx & (1 << 11))
#define CPU_FEAT_AVX512F      (ebx & (1 << 16))
#define CPU_FEAT_AVX512DQ     (ebx & (1 << 17))
#define CPU_FEAT_RDSEED       (ebx & (1 << 18))
#define CPU_FEAT_ADX          (ebx & (1 << 19))
#define CPU_FEAT_AVX512IFMA   (ebx & (1 << 21))
#define CPU_FEAT_CLFLUSHOPT   (ebx & (1 << 23))
#define CPU_FEAT_CLWB         (ebx & (1 << 24))
#define CPU_FEAT_AVX512PF     (ebx & (1 << 26))
#define CPU_FEAT_AVX512ER     (ebx & (1 << 27))
#define CPU_FEAT_AVX512CD     (ebx & (1 << 28))
#define CPU_FEAT_SHA          (ebx & (1 << 29))

#define CPU_HAS(feat_macro, reg_val)  ((feat_macro) & (reg_val))

// ======================================
// CPU SIGNATURES
// ======================================

typedef struct {
    u8 stepping;
    u8 model;
    u8 family;
    u8 proc_type;
    u8 ext_model;
    u8 ext_family;
} cpuid_signature_t;

cpuid_signature_t cpuid_signature(u32* _eax);

// ======================================
// ADDRESS WIDTH
// ======================================

void cpuid_addrwidth(u8* phys_addr_bits, u8* virt_addr_bits, u32* _eax);

// ----------------------------------------------------

#include "kernel.h"

#define BRAND_QEMU  1
#define BRAND_VBOX  2

void cpuid_simple(u32 eax_in, u32 ecx_in, u32 *eax, u32 *ebx, u32 *ecx_out, u32 *edx);
void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
int cpuid_info(int print);

#endif
