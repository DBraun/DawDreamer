#include "llvm/Frontend/OpenACC/ACC.h.inc"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace acc;

Directive llvm::acc::getOpenACCDirectiveKind(llvm::StringRef Str) {
  return llvm::StringSwitch<Directive>(Str)
    .Case("atomic",ACCD_atomic)
    .Case("cache",ACCD_cache)
    .Case("data",ACCD_data)
    .Case("declare",ACCD_declare)
    .Case("enter data",ACCD_enter_data)
    .Case("exit data",ACCD_exit_data)
    .Case("host_data",ACCD_host_data)
    .Case("init",ACCD_init)
    .Case("kernels",ACCD_kernels)
    .Case("kernels loop",ACCD_kernels_loop)
    .Case("loop",ACCD_loop)
    .Case("parallel",ACCD_parallel)
    .Case("parallel loop",ACCD_parallel_loop)
    .Case("routine",ACCD_routine)
    .Case("serial",ACCD_serial)
    .Case("serial loop",ACCD_serial_loop)
    .Case("set",ACCD_set)
    .Case("shutdown",ACCD_shutdown)
    .Case("unknown",ACCD_unknown)
    .Case("update",ACCD_update)
    .Case("wait",ACCD_wait)
    .Default(ACCD_unknown);
}

llvm::StringRef llvm::acc::getOpenACCDirectiveName(Directive Kind) {
  switch (Kind) {
    case ACCD_atomic:
      return "atomic";
    case ACCD_cache:
      return "cache";
    case ACCD_data:
      return "data";
    case ACCD_declare:
      return "declare";
    case ACCD_enter_data:
      return "enter data";
    case ACCD_exit_data:
      return "exit data";
    case ACCD_host_data:
      return "host_data";
    case ACCD_init:
      return "init";
    case ACCD_kernels:
      return "kernels";
    case ACCD_kernels_loop:
      return "kernels loop";
    case ACCD_loop:
      return "loop";
    case ACCD_parallel:
      return "parallel";
    case ACCD_parallel_loop:
      return "parallel loop";
    case ACCD_routine:
      return "routine";
    case ACCD_serial:
      return "serial";
    case ACCD_serial_loop:
      return "serial loop";
    case ACCD_set:
      return "set";
    case ACCD_shutdown:
      return "shutdown";
    case ACCD_unknown:
      return "unknown";
    case ACCD_update:
      return "update";
    case ACCD_wait:
      return "wait";
  }
  llvm_unreachable("Invalid OpenACC Directive kind");
}

Clause llvm::acc::getOpenACCClauseKind(llvm::StringRef Str) {
  return llvm::StringSwitch<Clause>(Str)
    .Case("async",ACCC_async)
    .Case("attach",ACCC_attach)
    .Case("auto",ACCC_auto)
    .Case("bind",ACCC_bind)
    .Case("capture",ACCC_capture)
    .Case("collapse",ACCC_collapse)
    .Case("copy",ACCC_copy)
    .Case("copyin",ACCC_copyin)
    .Case("copyout",ACCC_copyout)
    .Case("create",ACCC_create)
    .Case("default",ACCC_default)
    .Case("default_async",ACCC_default_async)
    .Case("delete",ACCC_delete)
    .Case("detach",ACCC_detach)
    .Case("device",ACCC_device)
    .Case("device_num",ACCC_device_num)
    .Case("deviceptr",ACCC_deviceptr)
    .Case("device_resident",ACCC_device_resident)
    .Case("device_type",ACCC_device_type)
    .Case("finalize",ACCC_finalize)
    .Case("firstprivate",ACCC_firstprivate)
    .Case("gang",ACCC_gang)
    .Case("host",ACCC_host)
    .Case("if",ACCC_if)
    .Case("if_present",ACCC_if_present)
    .Case("independent",ACCC_independent)
    .Case("link",ACCC_link)
    .Case("no_create",ACCC_no_create)
    .Case("nohost",ACCC_nohost)
    .Case("num_gangs",ACCC_num_gangs)
    .Case("num_workers",ACCC_num_workers)
    .Case("present",ACCC_present)
    .Case("private",ACCC_private)
    .Case("read",ACCC_read)
    .Case("reduction",ACCC_reduction)
    .Case("self",ACCC_self)
    .Case("seq",ACCC_seq)
    .Case("tile",ACCC_tile)
    .Case("unknown",ACCC_unknown)
    .Case("use_device",ACCC_use_device)
    .Case("vector",ACCC_vector)
    .Case("vector_length",ACCC_vector_length)
    .Case("wait",ACCC_wait)
    .Case("worker",ACCC_worker)
    .Case("write",ACCC_write)
    .Default(ACCC_unknown);
}

llvm::StringRef llvm::acc::getOpenACCClauseName(Clause Kind) {
  switch (Kind) {
    case ACCC_async:
      return "async";
    case ACCC_attach:
      return "attach";
    case ACCC_auto:
      return "auto";
    case ACCC_bind:
      return "bind";
    case ACCC_capture:
      return "capture";
    case ACCC_collapse:
      return "collapse";
    case ACCC_copy:
      return "copy";
    case ACCC_copyin:
      return "copyin";
    case ACCC_copyout:
      return "copyout";
    case ACCC_create:
      return "create";
    case ACCC_default:
      return "default";
    case ACCC_default_async:
      return "default_async";
    case ACCC_delete:
      return "delete";
    case ACCC_detach:
      return "detach";
    case ACCC_device:
      return "device";
    case ACCC_device_num:
      return "device_num";
    case ACCC_deviceptr:
      return "deviceptr";
    case ACCC_device_resident:
      return "device_resident";
    case ACCC_device_type:
      return "device_type";
    case ACCC_finalize:
      return "finalize";
    case ACCC_firstprivate:
      return "firstprivate";
    case ACCC_gang:
      return "gang";
    case ACCC_host:
      return "host";
    case ACCC_if:
      return "if";
    case ACCC_if_present:
      return "if_present";
    case ACCC_independent:
      return "independent";
    case ACCC_link:
      return "link";
    case ACCC_no_create:
      return "no_create";
    case ACCC_nohost:
      return "nohost";
    case ACCC_num_gangs:
      return "num_gangs";
    case ACCC_num_workers:
      return "num_workers";
    case ACCC_present:
      return "present";
    case ACCC_private:
      return "private";
    case ACCC_read:
      return "read";
    case ACCC_reduction:
      return "reduction";
    case ACCC_self:
      return "self";
    case ACCC_seq:
      return "seq";
    case ACCC_tile:
      return "tile";
    case ACCC_unknown:
      return "unknown";
    case ACCC_use_device:
      return "use_device";
    case ACCC_vector:
      return "vector";
    case ACCC_vector_length:
      return "vector_length";
    case ACCC_wait:
      return "wait";
    case ACCC_worker:
      return "worker";
    case ACCC_write:
      return "write";
  }
  llvm_unreachable("Invalid OpenACC Clause kind");
}

DefaultValue llvm::acc::getDefaultValue(llvm::StringRef Str) {
  return llvm::StringSwitch<DefaultValue>(Str)
    .Case("present",ACC_Default_present)
    .Case("none",ACC_Default_none)
    .Default(ACC_Default_none);
}

llvm::StringRef llvm::acc::getOpenACCDefaultValueName(llvm::acc::DefaultValue x) {
  switch (x) {
    case ACC_Default_present:
      return "present";
    case ACC_Default_none:
      return "none";
  }
  llvm_unreachable("Invalid OpenACC DefaultValue kind");
}

bool llvm::acc::isAllowedClauseForDirective(Directive D, Clause C, unsigned Version) {
  assert(unsigned(D) <= llvm::acc::Directive_enumSize);
  assert(unsigned(C) <= llvm::acc::Clause_enumSize);
  switch (D) {
    case ACCD_atomic:
      return false;
      break;
    case ACCD_cache:
      return false;
      break;
    case ACCD_data:
      switch (C) {
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_declare:
      switch (C) {
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_resident:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_link:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_enter_data:
      switch (C) {
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_exit_data:
      switch (C) {
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_finalize:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_delete:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_detach:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_host_data:
      switch (C) {
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_use_device:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_init:
      switch (C) {
        case ACCC_device_num:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_kernels:
      switch (C) {
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_gangs:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_workers:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector_length:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_kernels_loop:
      switch (C) {
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_gang:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_gangs:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_workers:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_tile:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector_length:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_worker:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_auto:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_independent:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_seq:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_loop:
      switch (C) {
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_gang:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_tile:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_worker:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_auto:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_independent:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_seq:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_parallel:
      switch (C) {
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_gangs:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_workers:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector_length:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_parallel_loop:
      switch (C) {
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_tile:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_gang:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_gangs:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_num_workers:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector_length:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_worker:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_auto:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_independent:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_seq:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_routine:
      switch (C) {
        case ACCC_bind:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_nohost:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_gang:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_seq:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_worker:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_serial:
      switch (C) {
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_serial_loop:
      switch (C) {
        case ACCC_attach:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copy:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_copyout:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_deviceptr:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_no_create:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_private:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_default:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_gang:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_tile:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_vector:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_worker:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_auto:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_independent:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_seq:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_set:
      switch (C) {
        case ACCC_default_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_num:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_shutdown:
      switch (C) {
        case ACCC_device_num:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_unknown:
      return false;
      break;
    case ACCD_update:
      switch (C) {
        case ACCC_device_type:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_wait:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if_present:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_device:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_host:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_self:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case ACCD_wait:
      switch (C) {
        case ACCC_async:
          return 1 <= Version && 2147483647 >= Version;
        case ACCC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
  }
  llvm_unreachable("Invalid OpenACC Directive kind");
}
