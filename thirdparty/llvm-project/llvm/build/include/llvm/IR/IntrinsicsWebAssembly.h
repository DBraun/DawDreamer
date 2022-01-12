/*===- TableGen'erated file -------------------------------------*- C++ -*-===*\
|*                                                                            *|
|* Intrinsic Function Source Fragment                                         *|
|*                                                                            *|
|* Automatically generated file, do not edit!                                 *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#ifndef LLVM_IR_INTRINSIC_WASM_ENUMS_H
#define LLVM_IR_INTRINSIC_WASM_ENUMS_H

namespace llvm {
namespace Intrinsic {
enum WASMIntrinsics : unsigned {
// Enum values for intrinsics
    wasm_alltrue = 8590,                              // llvm.wasm.alltrue
    wasm_anytrue,                              // llvm.wasm.anytrue
    wasm_avgr_unsigned,                        // llvm.wasm.avgr.unsigned
    wasm_bitmask,                              // llvm.wasm.bitmask
    wasm_bitselect,                            // llvm.wasm.bitselect
    wasm_catch,                                // llvm.wasm.catch
    wasm_ceil,                                 // llvm.wasm.ceil
    wasm_convert_low_signed,                   // llvm.wasm.convert.low.signed
    wasm_convert_low_unsigned,                 // llvm.wasm.convert.low.unsigned
    wasm_demote_zero,                          // llvm.wasm.demote.zero
    wasm_dot,                                  // llvm.wasm.dot
    wasm_eq,                                   // llvm.wasm.eq
    wasm_extadd_pairwise_signed,               // llvm.wasm.extadd.pairwise.signed
    wasm_extadd_pairwise_unsigned,             // llvm.wasm.extadd.pairwise.unsigned
    wasm_extmul_high_signed,                   // llvm.wasm.extmul.high.signed
    wasm_extmul_high_unsigned,                 // llvm.wasm.extmul.high.unsigned
    wasm_extmul_low_signed,                    // llvm.wasm.extmul.low.signed
    wasm_extmul_low_unsigned,                  // llvm.wasm.extmul.low.unsigned
    wasm_floor,                                // llvm.wasm.floor
    wasm_get_ehselector,                       // llvm.wasm.get.ehselector
    wasm_get_exception,                        // llvm.wasm.get.exception
    wasm_landingpad_index,                     // llvm.wasm.landingpad.index
    wasm_load16_lane,                          // llvm.wasm.load16.lane
    wasm_load32_lane,                          // llvm.wasm.load32.lane
    wasm_load32_zero,                          // llvm.wasm.load32.zero
    wasm_load64_lane,                          // llvm.wasm.load64.lane
    wasm_load64_zero,                          // llvm.wasm.load64.zero
    wasm_load8_lane,                           // llvm.wasm.load8.lane
    wasm_lsda,                                 // llvm.wasm.lsda
    wasm_memory_atomic_notify,                 // llvm.wasm.memory.atomic.notify
    wasm_memory_atomic_wait32,                 // llvm.wasm.memory.atomic.wait32
    wasm_memory_atomic_wait64,                 // llvm.wasm.memory.atomic.wait64
    wasm_memory_grow,                          // llvm.wasm.memory.grow
    wasm_memory_size,                          // llvm.wasm.memory.size
    wasm_narrow_signed,                        // llvm.wasm.narrow.signed
    wasm_narrow_unsigned,                      // llvm.wasm.narrow.unsigned
    wasm_nearest,                              // llvm.wasm.nearest
    wasm_pmax,                                 // llvm.wasm.pmax
    wasm_pmin,                                 // llvm.wasm.pmin
    wasm_popcnt,                               // llvm.wasm.popcnt
    wasm_prefetch_nt,                          // llvm.wasm.prefetch.nt
    wasm_prefetch_t,                           // llvm.wasm.prefetch.t
    wasm_promote_low,                          // llvm.wasm.promote.low
    wasm_q15mulr_saturate_signed,              // llvm.wasm.q15mulr.saturate.signed
    wasm_qfma,                                 // llvm.wasm.qfma
    wasm_qfms,                                 // llvm.wasm.qfms
    wasm_rethrow,                              // llvm.wasm.rethrow
    wasm_shuffle,                              // llvm.wasm.shuffle
    wasm_signselect,                           // llvm.wasm.signselect
    wasm_store16_lane,                         // llvm.wasm.store16.lane
    wasm_store32_lane,                         // llvm.wasm.store32.lane
    wasm_store64_lane,                         // llvm.wasm.store64.lane
    wasm_store8_lane,                          // llvm.wasm.store8.lane
    wasm_sub_saturate_signed,                  // llvm.wasm.sub.saturate.signed
    wasm_sub_saturate_unsigned,                // llvm.wasm.sub.saturate.unsigned
    wasm_swizzle,                              // llvm.wasm.swizzle
    wasm_throw,                                // llvm.wasm.throw
    wasm_tls_align,                            // llvm.wasm.tls.align
    wasm_tls_base,                             // llvm.wasm.tls.base
    wasm_tls_size,                             // llvm.wasm.tls.size
    wasm_trunc,                                // llvm.wasm.trunc
    wasm_trunc_saturate_signed,                // llvm.wasm.trunc.saturate.signed
    wasm_trunc_saturate_unsigned,              // llvm.wasm.trunc.saturate.unsigned
    wasm_trunc_saturate_zero_signed,           // llvm.wasm.trunc.saturate.zero.signed
    wasm_trunc_saturate_zero_unsigned,         // llvm.wasm.trunc.saturate.zero.unsigned
    wasm_trunc_signed,                         // llvm.wasm.trunc.signed
    wasm_trunc_unsigned,                       // llvm.wasm.trunc.unsigned
    wasm_widen_high_signed,                    // llvm.wasm.widen.high.signed
    wasm_widen_high_unsigned,                  // llvm.wasm.widen.high.unsigned
    wasm_widen_low_signed,                     // llvm.wasm.widen.low.signed
    wasm_widen_low_unsigned,                   // llvm.wasm.widen.low.unsigned
}; // enum
} // namespace Intrinsic
} // namespace llvm

#endif
