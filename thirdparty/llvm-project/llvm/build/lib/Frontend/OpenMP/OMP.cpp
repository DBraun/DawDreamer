#include "llvm/Frontend/OpenMP/OMP.h.inc"

#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
using namespace omp;

Directive llvm::omp::getOpenMPDirectiveKind(llvm::StringRef Str) {
  return llvm::StringSwitch<Directive>(Str)
    .Case("allocate",OMPD_allocate)
    .Case("assumes",OMPD_assumes)
    .Case("atomic",OMPD_atomic)
    .Case("barrier",OMPD_barrier)
    .Case("begin assumes",OMPD_begin_assumes)
    .Case("begin declare variant",OMPD_begin_declare_variant)
    .Case("cancel",OMPD_cancel)
    .Case("cancellation point",OMPD_cancellation_point)
    .Case("critical",OMPD_critical)
    .Case("declare mapper",OMPD_declare_mapper)
    .Case("declare reduction",OMPD_declare_reduction)
    .Case("declare simd",OMPD_declare_simd)
    .Case("declare target",OMPD_declare_target)
    .Case("declare variant",OMPD_declare_variant)
    .Case("depobj",OMPD_depobj)
    .Case("distribute",OMPD_distribute)
    .Case("distribute parallel do",OMPD_distribute_parallel_do)
    .Case("distribute parallel do simd",OMPD_distribute_parallel_do_simd)
    .Case("distribute parallel for",OMPD_distribute_parallel_for)
    .Case("distribute parallel for simd",OMPD_distribute_parallel_for_simd)
    .Case("distribute simd",OMPD_distribute_simd)
    .Case("do",OMPD_do)
    .Case("do simd",OMPD_do_simd)
    .Case("end assumes",OMPD_end_assumes)
    .Case("end declare target",OMPD_end_declare_target)
    .Case("end declare variant",OMPD_end_declare_variant)
    .Case("end do",OMPD_end_do)
    .Case("end do simd",OMPD_end_do_simd)
    .Case("end sections",OMPD_end_sections)
    .Case("end single",OMPD_end_single)
    .Case("end workshare",OMPD_end_workshare)
    .Case("flush",OMPD_flush)
    .Case("for",OMPD_for)
    .Case("for simd",OMPD_for_simd)
    .Case("master",OMPD_master)
    .Case("master taskloop",OMPD_master_taskloop)
    .Case("master taskloop simd",OMPD_master_taskloop_simd)
    .Case("ordered",OMPD_ordered)
    .Case("parallel",OMPD_parallel)
    .Case("parallel do",OMPD_parallel_do)
    .Case("parallel do simd",OMPD_parallel_do_simd)
    .Case("parallel for",OMPD_parallel_for)
    .Case("parallel for simd",OMPD_parallel_for_simd)
    .Case("parallel master",OMPD_parallel_master)
    .Case("parallel master taskloop",OMPD_parallel_master_taskloop)
    .Case("parallel master taskloop simd",OMPD_parallel_master_taskloop_simd)
    .Case("parallel sections",OMPD_parallel_sections)
    .Case("parallel workshare",OMPD_parallel_workshare)
    .Case("requires",OMPD_requires)
    .Case("scan",OMPD_scan)
    .Case("section",OMPD_section)
    .Case("sections",OMPD_sections)
    .Case("simd",OMPD_simd)
    .Case("single",OMPD_single)
    .Case("target",OMPD_target)
    .Case("target data",OMPD_target_data)
    .Case("target enter data",OMPD_target_enter_data)
    .Case("target exit data",OMPD_target_exit_data)
    .Case("target parallel",OMPD_target_parallel)
    .Case("target parallel do",OMPD_target_parallel_do)
    .Case("target parallel do simd",OMPD_target_parallel_do_simd)
    .Case("target parallel for",OMPD_target_parallel_for)
    .Case("target parallel for simd",OMPD_target_parallel_for_simd)
    .Case("target simd",OMPD_target_simd)
    .Case("target teams",OMPD_target_teams)
    .Case("target teams distribute",OMPD_target_teams_distribute)
    .Case("target teams distribute parallel do",OMPD_target_teams_distribute_parallel_do)
    .Case("target teams distribute parallel do simd",OMPD_target_teams_distribute_parallel_do_simd)
    .Case("target teams distribute parallel for",OMPD_target_teams_distribute_parallel_for)
    .Case("target teams distribute parallel for simd",OMPD_target_teams_distribute_parallel_for_simd)
    .Case("target teams distribute simd",OMPD_target_teams_distribute_simd)
    .Case("target update",OMPD_target_update)
    .Case("task",OMPD_task)
    .Case("taskgroup",OMPD_taskgroup)
    .Case("taskloop",OMPD_taskloop)
    .Case("taskloop simd",OMPD_taskloop_simd)
    .Case("taskwait",OMPD_taskwait)
    .Case("taskyield",OMPD_taskyield)
    .Case("teams",OMPD_teams)
    .Case("teams distribute",OMPD_teams_distribute)
    .Case("teams distribute parallel do",OMPD_teams_distribute_parallel_do)
    .Case("teams distribute parallel do simd",OMPD_teams_distribute_parallel_do_simd)
    .Case("teams distribute parallel for",OMPD_teams_distribute_parallel_for)
    .Case("teams distribute parallel for simd",OMPD_teams_distribute_parallel_for_simd)
    .Case("teams distribute simd",OMPD_teams_distribute_simd)
    .Case("threadprivate",OMPD_threadprivate)
    .Case("unknown",OMPD_unknown)
    .Case("workshare",OMPD_workshare)
    .Default(OMPD_unknown);
}

llvm::StringRef llvm::omp::getOpenMPDirectiveName(Directive Kind) {
  switch (Kind) {
    case OMPD_allocate:
      return "allocate";
    case OMPD_assumes:
      return "assumes";
    case OMPD_atomic:
      return "atomic";
    case OMPD_barrier:
      return "barrier";
    case OMPD_begin_assumes:
      return "begin assumes";
    case OMPD_begin_declare_variant:
      return "begin declare variant";
    case OMPD_cancel:
      return "cancel";
    case OMPD_cancellation_point:
      return "cancellation point";
    case OMPD_critical:
      return "critical";
    case OMPD_declare_mapper:
      return "declare mapper";
    case OMPD_declare_reduction:
      return "declare reduction";
    case OMPD_declare_simd:
      return "declare simd";
    case OMPD_declare_target:
      return "declare target";
    case OMPD_declare_variant:
      return "declare variant";
    case OMPD_depobj:
      return "depobj";
    case OMPD_distribute:
      return "distribute";
    case OMPD_distribute_parallel_do:
      return "distribute parallel do";
    case OMPD_distribute_parallel_do_simd:
      return "distribute parallel do simd";
    case OMPD_distribute_parallel_for:
      return "distribute parallel for";
    case OMPD_distribute_parallel_for_simd:
      return "distribute parallel for simd";
    case OMPD_distribute_simd:
      return "distribute simd";
    case OMPD_do:
      return "do";
    case OMPD_do_simd:
      return "do simd";
    case OMPD_end_assumes:
      return "end assumes";
    case OMPD_end_declare_target:
      return "end declare target";
    case OMPD_end_declare_variant:
      return "end declare variant";
    case OMPD_end_do:
      return "end do";
    case OMPD_end_do_simd:
      return "end do simd";
    case OMPD_end_sections:
      return "end sections";
    case OMPD_end_single:
      return "end single";
    case OMPD_end_workshare:
      return "end workshare";
    case OMPD_flush:
      return "flush";
    case OMPD_for:
      return "for";
    case OMPD_for_simd:
      return "for simd";
    case OMPD_master:
      return "master";
    case OMPD_master_taskloop:
      return "master taskloop";
    case OMPD_master_taskloop_simd:
      return "master taskloop simd";
    case OMPD_ordered:
      return "ordered";
    case OMPD_parallel:
      return "parallel";
    case OMPD_parallel_do:
      return "parallel do";
    case OMPD_parallel_do_simd:
      return "parallel do simd";
    case OMPD_parallel_for:
      return "parallel for";
    case OMPD_parallel_for_simd:
      return "parallel for simd";
    case OMPD_parallel_master:
      return "parallel master";
    case OMPD_parallel_master_taskloop:
      return "parallel master taskloop";
    case OMPD_parallel_master_taskloop_simd:
      return "parallel master taskloop simd";
    case OMPD_parallel_sections:
      return "parallel sections";
    case OMPD_parallel_workshare:
      return "parallel workshare";
    case OMPD_requires:
      return "requires";
    case OMPD_scan:
      return "scan";
    case OMPD_section:
      return "section";
    case OMPD_sections:
      return "sections";
    case OMPD_simd:
      return "simd";
    case OMPD_single:
      return "single";
    case OMPD_target:
      return "target";
    case OMPD_target_data:
      return "target data";
    case OMPD_target_enter_data:
      return "target enter data";
    case OMPD_target_exit_data:
      return "target exit data";
    case OMPD_target_parallel:
      return "target parallel";
    case OMPD_target_parallel_do:
      return "target parallel do";
    case OMPD_target_parallel_do_simd:
      return "target parallel do simd";
    case OMPD_target_parallel_for:
      return "target parallel for";
    case OMPD_target_parallel_for_simd:
      return "target parallel for simd";
    case OMPD_target_simd:
      return "target simd";
    case OMPD_target_teams:
      return "target teams";
    case OMPD_target_teams_distribute:
      return "target teams distribute";
    case OMPD_target_teams_distribute_parallel_do:
      return "target teams distribute parallel do";
    case OMPD_target_teams_distribute_parallel_do_simd:
      return "target teams distribute parallel do simd";
    case OMPD_target_teams_distribute_parallel_for:
      return "target teams distribute parallel for";
    case OMPD_target_teams_distribute_parallel_for_simd:
      return "target teams distribute parallel for simd";
    case OMPD_target_teams_distribute_simd:
      return "target teams distribute simd";
    case OMPD_target_update:
      return "target update";
    case OMPD_task:
      return "task";
    case OMPD_taskgroup:
      return "taskgroup";
    case OMPD_taskloop:
      return "taskloop";
    case OMPD_taskloop_simd:
      return "taskloop simd";
    case OMPD_taskwait:
      return "taskwait";
    case OMPD_taskyield:
      return "taskyield";
    case OMPD_teams:
      return "teams";
    case OMPD_teams_distribute:
      return "teams distribute";
    case OMPD_teams_distribute_parallel_do:
      return "teams distribute parallel do";
    case OMPD_teams_distribute_parallel_do_simd:
      return "teams distribute parallel do simd";
    case OMPD_teams_distribute_parallel_for:
      return "teams distribute parallel for";
    case OMPD_teams_distribute_parallel_for_simd:
      return "teams distribute parallel for simd";
    case OMPD_teams_distribute_simd:
      return "teams distribute simd";
    case OMPD_threadprivate:
      return "threadprivate";
    case OMPD_unknown:
      return "unknown";
    case OMPD_workshare:
      return "workshare";
  }
  llvm_unreachable("Invalid OpenMP Directive kind");
}

Clause llvm::omp::getOpenMPClauseKind(llvm::StringRef Str) {
  return llvm::StringSwitch<Clause>(Str)
    .Case("acq_rel",OMPC_acq_rel)
    .Case("acquire",OMPC_acquire)
    .Case("affinity",OMPC_affinity)
    .Case("aligned",OMPC_aligned)
    .Case("allocate",OMPC_allocate)
    .Case("allocator",OMPC_allocator)
    .Case("atomic_default_mem_order",OMPC_atomic_default_mem_order)
    .Case("capture",OMPC_capture)
    .Case("collapse",OMPC_collapse)
    .Case("copyprivate",OMPC_copyprivate)
    .Case("copyin",OMPC_copyin)
    .Case("default",OMPC_default)
    .Case("defaultmap",OMPC_defaultmap)
    .Case("depend",OMPC_depend)
    .Case("depobj",OMPC_unknown)
    .Case("destroy",OMPC_destroy)
    .Case("detach",OMPC_detach)
    .Case("device",OMPC_device)
    .Case("device_type",OMPC_device_type)
    .Case("dist_schedule",OMPC_dist_schedule)
    .Case("dynamic_allocators",OMPC_dynamic_allocators)
    .Case("exclusive",OMPC_exclusive)
    .Case("final",OMPC_final)
    .Case("firstprivate",OMPC_firstprivate)
    .Case("flush",OMPC_unknown)
    .Case("from",OMPC_from)
    .Case("grainsize",OMPC_grainsize)
    .Case("hint",OMPC_hint)
    .Case("if",OMPC_if)
    .Case("in_reduction",OMPC_in_reduction)
    .Case("inbranch",OMPC_inbranch)
    .Case("inclusive",OMPC_inclusive)
    .Case("is_device_ptr",OMPC_is_device_ptr)
    .Case("lastprivate",OMPC_lastprivate)
    .Case("linear",OMPC_linear)
    .Case("link",OMPC_link)
    .Case("map",OMPC_map)
    .Case("match",OMPC_match)
    .Case("mergeable",OMPC_mergeable)
    .Case("nogroup",OMPC_nogroup)
    .Case("nowait",OMPC_nowait)
    .Case("nontemporal",OMPC_nontemporal)
    .Case("notinbranch",OMPC_notinbranch)
    .Case("num_tasks",OMPC_num_tasks)
    .Case("num_teams",OMPC_num_teams)
    .Case("num_threads",OMPC_num_threads)
    .Case("order",OMPC_order)
    .Case("ordered",OMPC_ordered)
    .Case("priority",OMPC_priority)
    .Case("private",OMPC_private)
    .Case("proc_bind",OMPC_proc_bind)
    .Case("read",OMPC_read)
    .Case("reduction",OMPC_reduction)
    .Case("relaxed",OMPC_relaxed)
    .Case("release",OMPC_release)
    .Case("reverse_offload",OMPC_reverse_offload)
    .Case("safelen",OMPC_safelen)
    .Case("schedule",OMPC_schedule)
    .Case("seq_cst",OMPC_seq_cst)
    .Case("shared",OMPC_shared)
    .Case("simd",OMPC_simd)
    .Case("simdlen",OMPC_simdlen)
    .Case("task_reduction",OMPC_task_reduction)
    .Case("thread_limit",OMPC_thread_limit)
    .Case("threadprivate",OMPC_unknown)
    .Case("threads",OMPC_threads)
    .Case("to",OMPC_to)
    .Case("unified_address",OMPC_unified_address)
    .Case("unified_shared_memory",OMPC_unified_shared_memory)
    .Case("uniform",OMPC_uniform)
    .Case("unknown",OMPC_unknown)
    .Case("untied",OMPC_untied)
    .Case("update",OMPC_update)
    .Case("use_device_addr",OMPC_use_device_addr)
    .Case("use_device_ptr",OMPC_use_device_ptr)
    .Case("uses_allocators",OMPC_uses_allocators)
    .Case("write",OMPC_write)
    .Default(OMPC_unknown);
}

llvm::StringRef llvm::omp::getOpenMPClauseName(Clause Kind) {
  switch (Kind) {
    case OMPC_acq_rel:
      return "acq_rel";
    case OMPC_acquire:
      return "acquire";
    case OMPC_affinity:
      return "affinity";
    case OMPC_aligned:
      return "aligned";
    case OMPC_allocate:
      return "allocate";
    case OMPC_allocator:
      return "allocator";
    case OMPC_atomic_default_mem_order:
      return "atomic_default_mem_order";
    case OMPC_capture:
      return "capture";
    case OMPC_collapse:
      return "collapse";
    case OMPC_copyprivate:
      return "copyprivate";
    case OMPC_copyin:
      return "copyin";
    case OMPC_default:
      return "default";
    case OMPC_defaultmap:
      return "defaultmap";
    case OMPC_depend:
      return "depend";
    case OMPC_depobj:
      return "depobj";
    case OMPC_destroy:
      return "destroy";
    case OMPC_detach:
      return "detach";
    case OMPC_device:
      return "device";
    case OMPC_device_type:
      return "device_type";
    case OMPC_dist_schedule:
      return "dist_schedule";
    case OMPC_dynamic_allocators:
      return "dynamic_allocators";
    case OMPC_exclusive:
      return "exclusive";
    case OMPC_final:
      return "final";
    case OMPC_firstprivate:
      return "firstprivate";
    case OMPC_flush:
      return "flush";
    case OMPC_from:
      return "from";
    case OMPC_grainsize:
      return "grainsize";
    case OMPC_hint:
      return "hint";
    case OMPC_if:
      return "if";
    case OMPC_in_reduction:
      return "in_reduction";
    case OMPC_inbranch:
      return "inbranch";
    case OMPC_inclusive:
      return "inclusive";
    case OMPC_is_device_ptr:
      return "is_device_ptr";
    case OMPC_lastprivate:
      return "lastprivate";
    case OMPC_linear:
      return "linear";
    case OMPC_link:
      return "link";
    case OMPC_map:
      return "map";
    case OMPC_match:
      return "match";
    case OMPC_mergeable:
      return "mergeable";
    case OMPC_nogroup:
      return "nogroup";
    case OMPC_nowait:
      return "nowait";
    case OMPC_nontemporal:
      return "nontemporal";
    case OMPC_notinbranch:
      return "notinbranch";
    case OMPC_num_tasks:
      return "num_tasks";
    case OMPC_num_teams:
      return "num_teams";
    case OMPC_num_threads:
      return "num_threads";
    case OMPC_order:
      return "order";
    case OMPC_ordered:
      return "ordered";
    case OMPC_priority:
      return "priority";
    case OMPC_private:
      return "private";
    case OMPC_proc_bind:
      return "proc_bind";
    case OMPC_read:
      return "read";
    case OMPC_reduction:
      return "reduction";
    case OMPC_relaxed:
      return "relaxed";
    case OMPC_release:
      return "release";
    case OMPC_reverse_offload:
      return "reverse_offload";
    case OMPC_safelen:
      return "safelen";
    case OMPC_schedule:
      return "schedule";
    case OMPC_seq_cst:
      return "seq_cst";
    case OMPC_shared:
      return "shared";
    case OMPC_simd:
      return "simd";
    case OMPC_simdlen:
      return "simdlen";
    case OMPC_task_reduction:
      return "task_reduction";
    case OMPC_thread_limit:
      return "thread_limit";
    case OMPC_threadprivate:
      return "threadprivate or thread local";
    case OMPC_threads:
      return "threads";
    case OMPC_to:
      return "to";
    case OMPC_unified_address:
      return "unified_address";
    case OMPC_unified_shared_memory:
      return "unified_shared_memory";
    case OMPC_uniform:
      return "uniform";
    case OMPC_unknown:
      return "unknown";
    case OMPC_untied:
      return "untied";
    case OMPC_update:
      return "update";
    case OMPC_use_device_addr:
      return "use_device_addr";
    case OMPC_use_device_ptr:
      return "use_device_ptr";
    case OMPC_uses_allocators:
      return "uses_allocators";
    case OMPC_write:
      return "write";
  }
  llvm_unreachable("Invalid OpenMP Clause kind");
}

OrderKind llvm::omp::getOrderKind(llvm::StringRef Str) {
  return llvm::StringSwitch<OrderKind>(Str)
    .Case("default",OMP_ORDER_concurrent)
    .Default(OMP_ORDER_concurrent);
}

llvm::StringRef llvm::omp::getOpenMPOrderKindName(llvm::omp::OrderKind x) {
  switch (x) {
    case OMP_ORDER_concurrent:
      return "default";
  }
  llvm_unreachable("Invalid OpenMP OrderKind kind");
}

ProcBindKind llvm::omp::getProcBindKind(llvm::StringRef Str) {
  return llvm::StringSwitch<ProcBindKind>(Str)
    .Case("master",OMP_PROC_BIND_master)
    .Case("close",OMP_PROC_BIND_close)
    .Case("spread",OMP_PROC_BIND_spread)
    .Case("default",OMP_PROC_BIND_default)
    .Case("unknown",OMP_PROC_BIND_unknown)
    .Default(OMP_PROC_BIND_unknown);
}

llvm::StringRef llvm::omp::getOpenMPProcBindKindName(llvm::omp::ProcBindKind x) {
  switch (x) {
    case OMP_PROC_BIND_master:
      return "master";
    case OMP_PROC_BIND_close:
      return "close";
    case OMP_PROC_BIND_spread:
      return "spread";
    case OMP_PROC_BIND_default:
      return "default";
    case OMP_PROC_BIND_unknown:
      return "unknown";
  }
  llvm_unreachable("Invalid OpenMP ProcBindKind kind");
}

ScheduleKind llvm::omp::getScheduleKind(llvm::StringRef Str) {
  return llvm::StringSwitch<ScheduleKind>(Str)
    .Case("Static",OMP_SCHEDULE_Static)
    .Case("Dynamic",OMP_SCHEDULE_Dynamic)
    .Case("Guided",OMP_SCHEDULE_Guided)
    .Case("Auto",OMP_SCHEDULE_Auto)
    .Case("Runtime",OMP_SCHEDULE_Runtime)
    .Case("Default",OMP_SCHEDULE_Default)
    .Default(OMP_SCHEDULE_Default);
}

llvm::StringRef llvm::omp::getOpenMPScheduleKindName(llvm::omp::ScheduleKind x) {
  switch (x) {
    case OMP_SCHEDULE_Static:
      return "Static";
    case OMP_SCHEDULE_Dynamic:
      return "Dynamic";
    case OMP_SCHEDULE_Guided:
      return "Guided";
    case OMP_SCHEDULE_Auto:
      return "Auto";
    case OMP_SCHEDULE_Runtime:
      return "Runtime";
    case OMP_SCHEDULE_Default:
      return "Default";
  }
  llvm_unreachable("Invalid OpenMP ScheduleKind kind");
}

bool llvm::omp::isAllowedClauseForDirective(Directive D, Clause C, unsigned Version) {
  assert(unsigned(D) <= llvm::omp::Directive_enumSize);
  assert(unsigned(C) <= llvm::omp::Clause_enumSize);
  switch (D) {
    case OMPD_allocate:
      switch (C) {
        case OMPC_allocator:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_assumes:
      return false;
      break;
    case OMPD_atomic:
      switch (C) {
        case OMPC_read:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_write:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_update:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_capture:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_seq_cst:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_acq_rel:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_acquire:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_release:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_relaxed:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_hint:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_barrier:
      return false;
      break;
    case OMPD_begin_assumes:
      return false;
      break;
    case OMPD_begin_declare_variant:
      return false;
      break;
    case OMPD_cancel:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_cancellation_point:
      return false;
      break;
    case OMPD_critical:
      switch (C) {
        case OMPC_hint:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_declare_mapper:
      switch (C) {
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_declare_reduction:
      return false;
      break;
    case OMPD_declare_simd:
      switch (C) {
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uniform:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_inbranch:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_notinbranch:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_declare_target:
      switch (C) {
        case OMPC_to:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_link:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_declare_variant:
      switch (C) {
        case OMPC_match:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_depobj:
      switch (C) {
        case OMPC_depend:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_destroy:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_update:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_depobj:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute_parallel_do:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute_parallel_do_simd:
      switch (C) {
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute_parallel_for:
      switch (C) {
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute_parallel_for_simd:
      switch (C) {
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_distribute_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_do:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_do_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_end_assumes:
      return false;
      break;
    case OMPD_end_declare_target:
      return false;
      break;
    case OMPD_end_declare_variant:
      return false;
      break;
    case OMPD_end_do:
      return false;
      break;
    case OMPD_end_do_simd:
      return false;
      break;
    case OMPD_end_sections:
      switch (C) {
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_end_single:
      switch (C) {
        case OMPC_copyprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_end_workshare:
      switch (C) {
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_flush:
      switch (C) {
        case OMPC_acq_rel:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_acquire:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_release:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_flush:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_for:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_for_simd:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_master:
      return false;
      break;
    case OMPD_master_taskloop:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_in_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_master_taskloop_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_in_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_ordered:
      switch (C) {
        case OMPC_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simd:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_do:
      switch (C) {
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_do_simd:
      switch (C) {
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_for:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_for_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_master:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_master_taskloop:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_master_taskloop_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_sections:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_parallel_workshare:
      switch (C) {
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_requires:
      switch (C) {
        case OMPC_unified_address:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_unified_shared_memory:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reverse_offload:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dynamic_allocators:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_atomic_default_mem_order:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_scan:
      switch (C) {
        case OMPC_inclusive:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_exclusive:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_section:
      return false;
      break;
    case OMPD_sections:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_simd:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_single:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_data:
      switch (C) {
        case OMPC_use_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_use_device_addr:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_enter_data:
      switch (C) {
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_exit_data:
      switch (C) {
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_parallel:
      switch (C) {
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_parallel_do:
      switch (C) {
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocator:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_parallel_do_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_parallel_for:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_parallel_for_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute_parallel_do:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute_parallel_do_simd:
      switch (C) {
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute_parallel_for:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute_parallel_for_simd:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_teams_distribute_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_is_device_ptr:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_map:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_uses_allocators:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_defaultmap:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_target_update:
      switch (C) {
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_device:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_to:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_from:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nowait:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_task:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_depend:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_in_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_detach:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_affinity:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_taskgroup:
      switch (C) {
        case OMPC_task_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_taskloop:
      switch (C) {
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_in_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_taskloop_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_in_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_mergeable:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nogroup:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_untied:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_final:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_priority:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_grainsize:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_tasks:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_taskwait:
      switch (C) {
        case OMPC_depend:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_taskyield:
      return false;
      break;
    case OMPD_teams:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute:
      switch (C) {
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute_parallel_do:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_ordered:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute_parallel_do_simd:
      switch (C) {
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute_parallel_for:
      switch (C) {
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_copyin:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute_parallel_for_simd:
      switch (C) {
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_threads:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_proc_bind:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_teams_distribute_simd:
      switch (C) {
        case OMPC_aligned:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_allocate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_firstprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_lastprivate:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_linear:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_nontemporal:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_order:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_private:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_reduction:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_shared:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_collapse:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_default:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_dist_schedule:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_if:
          return 50 <= Version && 2147483647 >= Version;
        case OMPC_num_teams:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_safelen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_simdlen:
          return 1 <= Version && 2147483647 >= Version;
        case OMPC_thread_limit:
          return 1 <= Version && 2147483647 >= Version;
        default:
          return false;
      }
      break;
    case OMPD_threadprivate:
      return false;
      break;
    case OMPD_unknown:
      return false;
      break;
    case OMPD_workshare:
      return false;
      break;
  }
  llvm_unreachable("Invalid OpenMP Directive kind");
}
