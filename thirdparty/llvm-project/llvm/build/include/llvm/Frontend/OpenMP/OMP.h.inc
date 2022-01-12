#ifndef LLVM_OpenMP_INC
#define LLVM_OpenMP_INC

#include "llvm/ADT/BitmaskEnum.h"

namespace llvm {
class StringRef;
namespace omp {

LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

enum class Directive {
  OMPD_allocate,
  OMPD_assumes,
  OMPD_atomic,
  OMPD_barrier,
  OMPD_begin_assumes,
  OMPD_begin_declare_variant,
  OMPD_cancel,
  OMPD_cancellation_point,
  OMPD_critical,
  OMPD_declare_mapper,
  OMPD_declare_reduction,
  OMPD_declare_simd,
  OMPD_declare_target,
  OMPD_declare_variant,
  OMPD_depobj,
  OMPD_distribute,
  OMPD_distribute_parallel_do,
  OMPD_distribute_parallel_do_simd,
  OMPD_distribute_parallel_for,
  OMPD_distribute_parallel_for_simd,
  OMPD_distribute_simd,
  OMPD_do,
  OMPD_do_simd,
  OMPD_end_assumes,
  OMPD_end_declare_target,
  OMPD_end_declare_variant,
  OMPD_end_do,
  OMPD_end_do_simd,
  OMPD_end_sections,
  OMPD_end_single,
  OMPD_end_workshare,
  OMPD_flush,
  OMPD_for,
  OMPD_for_simd,
  OMPD_master,
  OMPD_master_taskloop,
  OMPD_master_taskloop_simd,
  OMPD_ordered,
  OMPD_parallel,
  OMPD_parallel_do,
  OMPD_parallel_do_simd,
  OMPD_parallel_for,
  OMPD_parallel_for_simd,
  OMPD_parallel_master,
  OMPD_parallel_master_taskloop,
  OMPD_parallel_master_taskloop_simd,
  OMPD_parallel_sections,
  OMPD_parallel_workshare,
  OMPD_requires,
  OMPD_scan,
  OMPD_section,
  OMPD_sections,
  OMPD_simd,
  OMPD_single,
  OMPD_target,
  OMPD_target_data,
  OMPD_target_enter_data,
  OMPD_target_exit_data,
  OMPD_target_parallel,
  OMPD_target_parallel_do,
  OMPD_target_parallel_do_simd,
  OMPD_target_parallel_for,
  OMPD_target_parallel_for_simd,
  OMPD_target_simd,
  OMPD_target_teams,
  OMPD_target_teams_distribute,
  OMPD_target_teams_distribute_parallel_do,
  OMPD_target_teams_distribute_parallel_do_simd,
  OMPD_target_teams_distribute_parallel_for,
  OMPD_target_teams_distribute_parallel_for_simd,
  OMPD_target_teams_distribute_simd,
  OMPD_target_update,
  OMPD_task,
  OMPD_taskgroup,
  OMPD_taskloop,
  OMPD_taskloop_simd,
  OMPD_taskwait,
  OMPD_taskyield,
  OMPD_teams,
  OMPD_teams_distribute,
  OMPD_teams_distribute_parallel_do,
  OMPD_teams_distribute_parallel_do_simd,
  OMPD_teams_distribute_parallel_for,
  OMPD_teams_distribute_parallel_for_simd,
  OMPD_teams_distribute_simd,
  OMPD_threadprivate,
  OMPD_unknown,
  OMPD_workshare,
};

static constexpr std::size_t Directive_enumSize = 88;

constexpr auto OMPD_allocate = llvm::omp::Directive::OMPD_allocate;
constexpr auto OMPD_assumes = llvm::omp::Directive::OMPD_assumes;
constexpr auto OMPD_atomic = llvm::omp::Directive::OMPD_atomic;
constexpr auto OMPD_barrier = llvm::omp::Directive::OMPD_barrier;
constexpr auto OMPD_begin_assumes = llvm::omp::Directive::OMPD_begin_assumes;
constexpr auto OMPD_begin_declare_variant = llvm::omp::Directive::OMPD_begin_declare_variant;
constexpr auto OMPD_cancel = llvm::omp::Directive::OMPD_cancel;
constexpr auto OMPD_cancellation_point = llvm::omp::Directive::OMPD_cancellation_point;
constexpr auto OMPD_critical = llvm::omp::Directive::OMPD_critical;
constexpr auto OMPD_declare_mapper = llvm::omp::Directive::OMPD_declare_mapper;
constexpr auto OMPD_declare_reduction = llvm::omp::Directive::OMPD_declare_reduction;
constexpr auto OMPD_declare_simd = llvm::omp::Directive::OMPD_declare_simd;
constexpr auto OMPD_declare_target = llvm::omp::Directive::OMPD_declare_target;
constexpr auto OMPD_declare_variant = llvm::omp::Directive::OMPD_declare_variant;
constexpr auto OMPD_depobj = llvm::omp::Directive::OMPD_depobj;
constexpr auto OMPD_distribute = llvm::omp::Directive::OMPD_distribute;
constexpr auto OMPD_distribute_parallel_do = llvm::omp::Directive::OMPD_distribute_parallel_do;
constexpr auto OMPD_distribute_parallel_do_simd = llvm::omp::Directive::OMPD_distribute_parallel_do_simd;
constexpr auto OMPD_distribute_parallel_for = llvm::omp::Directive::OMPD_distribute_parallel_for;
constexpr auto OMPD_distribute_parallel_for_simd = llvm::omp::Directive::OMPD_distribute_parallel_for_simd;
constexpr auto OMPD_distribute_simd = llvm::omp::Directive::OMPD_distribute_simd;
constexpr auto OMPD_do = llvm::omp::Directive::OMPD_do;
constexpr auto OMPD_do_simd = llvm::omp::Directive::OMPD_do_simd;
constexpr auto OMPD_end_assumes = llvm::omp::Directive::OMPD_end_assumes;
constexpr auto OMPD_end_declare_target = llvm::omp::Directive::OMPD_end_declare_target;
constexpr auto OMPD_end_declare_variant = llvm::omp::Directive::OMPD_end_declare_variant;
constexpr auto OMPD_end_do = llvm::omp::Directive::OMPD_end_do;
constexpr auto OMPD_end_do_simd = llvm::omp::Directive::OMPD_end_do_simd;
constexpr auto OMPD_end_sections = llvm::omp::Directive::OMPD_end_sections;
constexpr auto OMPD_end_single = llvm::omp::Directive::OMPD_end_single;
constexpr auto OMPD_end_workshare = llvm::omp::Directive::OMPD_end_workshare;
constexpr auto OMPD_flush = llvm::omp::Directive::OMPD_flush;
constexpr auto OMPD_for = llvm::omp::Directive::OMPD_for;
constexpr auto OMPD_for_simd = llvm::omp::Directive::OMPD_for_simd;
constexpr auto OMPD_master = llvm::omp::Directive::OMPD_master;
constexpr auto OMPD_master_taskloop = llvm::omp::Directive::OMPD_master_taskloop;
constexpr auto OMPD_master_taskloop_simd = llvm::omp::Directive::OMPD_master_taskloop_simd;
constexpr auto OMPD_ordered = llvm::omp::Directive::OMPD_ordered;
constexpr auto OMPD_parallel = llvm::omp::Directive::OMPD_parallel;
constexpr auto OMPD_parallel_do = llvm::omp::Directive::OMPD_parallel_do;
constexpr auto OMPD_parallel_do_simd = llvm::omp::Directive::OMPD_parallel_do_simd;
constexpr auto OMPD_parallel_for = llvm::omp::Directive::OMPD_parallel_for;
constexpr auto OMPD_parallel_for_simd = llvm::omp::Directive::OMPD_parallel_for_simd;
constexpr auto OMPD_parallel_master = llvm::omp::Directive::OMPD_parallel_master;
constexpr auto OMPD_parallel_master_taskloop = llvm::omp::Directive::OMPD_parallel_master_taskloop;
constexpr auto OMPD_parallel_master_taskloop_simd = llvm::omp::Directive::OMPD_parallel_master_taskloop_simd;
constexpr auto OMPD_parallel_sections = llvm::omp::Directive::OMPD_parallel_sections;
constexpr auto OMPD_parallel_workshare = llvm::omp::Directive::OMPD_parallel_workshare;
constexpr auto OMPD_requires = llvm::omp::Directive::OMPD_requires;
constexpr auto OMPD_scan = llvm::omp::Directive::OMPD_scan;
constexpr auto OMPD_section = llvm::omp::Directive::OMPD_section;
constexpr auto OMPD_sections = llvm::omp::Directive::OMPD_sections;
constexpr auto OMPD_simd = llvm::omp::Directive::OMPD_simd;
constexpr auto OMPD_single = llvm::omp::Directive::OMPD_single;
constexpr auto OMPD_target = llvm::omp::Directive::OMPD_target;
constexpr auto OMPD_target_data = llvm::omp::Directive::OMPD_target_data;
constexpr auto OMPD_target_enter_data = llvm::omp::Directive::OMPD_target_enter_data;
constexpr auto OMPD_target_exit_data = llvm::omp::Directive::OMPD_target_exit_data;
constexpr auto OMPD_target_parallel = llvm::omp::Directive::OMPD_target_parallel;
constexpr auto OMPD_target_parallel_do = llvm::omp::Directive::OMPD_target_parallel_do;
constexpr auto OMPD_target_parallel_do_simd = llvm::omp::Directive::OMPD_target_parallel_do_simd;
constexpr auto OMPD_target_parallel_for = llvm::omp::Directive::OMPD_target_parallel_for;
constexpr auto OMPD_target_parallel_for_simd = llvm::omp::Directive::OMPD_target_parallel_for_simd;
constexpr auto OMPD_target_simd = llvm::omp::Directive::OMPD_target_simd;
constexpr auto OMPD_target_teams = llvm::omp::Directive::OMPD_target_teams;
constexpr auto OMPD_target_teams_distribute = llvm::omp::Directive::OMPD_target_teams_distribute;
constexpr auto OMPD_target_teams_distribute_parallel_do = llvm::omp::Directive::OMPD_target_teams_distribute_parallel_do;
constexpr auto OMPD_target_teams_distribute_parallel_do_simd = llvm::omp::Directive::OMPD_target_teams_distribute_parallel_do_simd;
constexpr auto OMPD_target_teams_distribute_parallel_for = llvm::omp::Directive::OMPD_target_teams_distribute_parallel_for;
constexpr auto OMPD_target_teams_distribute_parallel_for_simd = llvm::omp::Directive::OMPD_target_teams_distribute_parallel_for_simd;
constexpr auto OMPD_target_teams_distribute_simd = llvm::omp::Directive::OMPD_target_teams_distribute_simd;
constexpr auto OMPD_target_update = llvm::omp::Directive::OMPD_target_update;
constexpr auto OMPD_task = llvm::omp::Directive::OMPD_task;
constexpr auto OMPD_taskgroup = llvm::omp::Directive::OMPD_taskgroup;
constexpr auto OMPD_taskloop = llvm::omp::Directive::OMPD_taskloop;
constexpr auto OMPD_taskloop_simd = llvm::omp::Directive::OMPD_taskloop_simd;
constexpr auto OMPD_taskwait = llvm::omp::Directive::OMPD_taskwait;
constexpr auto OMPD_taskyield = llvm::omp::Directive::OMPD_taskyield;
constexpr auto OMPD_teams = llvm::omp::Directive::OMPD_teams;
constexpr auto OMPD_teams_distribute = llvm::omp::Directive::OMPD_teams_distribute;
constexpr auto OMPD_teams_distribute_parallel_do = llvm::omp::Directive::OMPD_teams_distribute_parallel_do;
constexpr auto OMPD_teams_distribute_parallel_do_simd = llvm::omp::Directive::OMPD_teams_distribute_parallel_do_simd;
constexpr auto OMPD_teams_distribute_parallel_for = llvm::omp::Directive::OMPD_teams_distribute_parallel_for;
constexpr auto OMPD_teams_distribute_parallel_for_simd = llvm::omp::Directive::OMPD_teams_distribute_parallel_for_simd;
constexpr auto OMPD_teams_distribute_simd = llvm::omp::Directive::OMPD_teams_distribute_simd;
constexpr auto OMPD_threadprivate = llvm::omp::Directive::OMPD_threadprivate;
constexpr auto OMPD_unknown = llvm::omp::Directive::OMPD_unknown;
constexpr auto OMPD_workshare = llvm::omp::Directive::OMPD_workshare;

enum class Clause {
  OMPC_acq_rel,
  OMPC_acquire,
  OMPC_affinity,
  OMPC_aligned,
  OMPC_allocate,
  OMPC_allocator,
  OMPC_atomic_default_mem_order,
  OMPC_capture,
  OMPC_collapse,
  OMPC_copyprivate,
  OMPC_copyin,
  OMPC_default,
  OMPC_defaultmap,
  OMPC_depend,
  OMPC_depobj,
  OMPC_destroy,
  OMPC_detach,
  OMPC_device,
  OMPC_device_type,
  OMPC_dist_schedule,
  OMPC_dynamic_allocators,
  OMPC_exclusive,
  OMPC_final,
  OMPC_firstprivate,
  OMPC_flush,
  OMPC_from,
  OMPC_grainsize,
  OMPC_hint,
  OMPC_if,
  OMPC_in_reduction,
  OMPC_inbranch,
  OMPC_inclusive,
  OMPC_is_device_ptr,
  OMPC_lastprivate,
  OMPC_linear,
  OMPC_link,
  OMPC_map,
  OMPC_match,
  OMPC_mergeable,
  OMPC_nogroup,
  OMPC_nowait,
  OMPC_nontemporal,
  OMPC_notinbranch,
  OMPC_num_tasks,
  OMPC_num_teams,
  OMPC_num_threads,
  OMPC_order,
  OMPC_ordered,
  OMPC_priority,
  OMPC_private,
  OMPC_proc_bind,
  OMPC_read,
  OMPC_reduction,
  OMPC_relaxed,
  OMPC_release,
  OMPC_reverse_offload,
  OMPC_safelen,
  OMPC_schedule,
  OMPC_seq_cst,
  OMPC_shared,
  OMPC_simd,
  OMPC_simdlen,
  OMPC_task_reduction,
  OMPC_thread_limit,
  OMPC_threadprivate,
  OMPC_threads,
  OMPC_to,
  OMPC_unified_address,
  OMPC_unified_shared_memory,
  OMPC_uniform,
  OMPC_unknown,
  OMPC_untied,
  OMPC_update,
  OMPC_use_device_addr,
  OMPC_use_device_ptr,
  OMPC_uses_allocators,
  OMPC_write,
};

static constexpr std::size_t Clause_enumSize = 77;

constexpr auto OMPC_acq_rel = llvm::omp::Clause::OMPC_acq_rel;
constexpr auto OMPC_acquire = llvm::omp::Clause::OMPC_acquire;
constexpr auto OMPC_affinity = llvm::omp::Clause::OMPC_affinity;
constexpr auto OMPC_aligned = llvm::omp::Clause::OMPC_aligned;
constexpr auto OMPC_allocate = llvm::omp::Clause::OMPC_allocate;
constexpr auto OMPC_allocator = llvm::omp::Clause::OMPC_allocator;
constexpr auto OMPC_atomic_default_mem_order = llvm::omp::Clause::OMPC_atomic_default_mem_order;
constexpr auto OMPC_capture = llvm::omp::Clause::OMPC_capture;
constexpr auto OMPC_collapse = llvm::omp::Clause::OMPC_collapse;
constexpr auto OMPC_copyprivate = llvm::omp::Clause::OMPC_copyprivate;
constexpr auto OMPC_copyin = llvm::omp::Clause::OMPC_copyin;
constexpr auto OMPC_default = llvm::omp::Clause::OMPC_default;
constexpr auto OMPC_defaultmap = llvm::omp::Clause::OMPC_defaultmap;
constexpr auto OMPC_depend = llvm::omp::Clause::OMPC_depend;
constexpr auto OMPC_depobj = llvm::omp::Clause::OMPC_depobj;
constexpr auto OMPC_destroy = llvm::omp::Clause::OMPC_destroy;
constexpr auto OMPC_detach = llvm::omp::Clause::OMPC_detach;
constexpr auto OMPC_device = llvm::omp::Clause::OMPC_device;
constexpr auto OMPC_device_type = llvm::omp::Clause::OMPC_device_type;
constexpr auto OMPC_dist_schedule = llvm::omp::Clause::OMPC_dist_schedule;
constexpr auto OMPC_dynamic_allocators = llvm::omp::Clause::OMPC_dynamic_allocators;
constexpr auto OMPC_exclusive = llvm::omp::Clause::OMPC_exclusive;
constexpr auto OMPC_final = llvm::omp::Clause::OMPC_final;
constexpr auto OMPC_firstprivate = llvm::omp::Clause::OMPC_firstprivate;
constexpr auto OMPC_flush = llvm::omp::Clause::OMPC_flush;
constexpr auto OMPC_from = llvm::omp::Clause::OMPC_from;
constexpr auto OMPC_grainsize = llvm::omp::Clause::OMPC_grainsize;
constexpr auto OMPC_hint = llvm::omp::Clause::OMPC_hint;
constexpr auto OMPC_if = llvm::omp::Clause::OMPC_if;
constexpr auto OMPC_in_reduction = llvm::omp::Clause::OMPC_in_reduction;
constexpr auto OMPC_inbranch = llvm::omp::Clause::OMPC_inbranch;
constexpr auto OMPC_inclusive = llvm::omp::Clause::OMPC_inclusive;
constexpr auto OMPC_is_device_ptr = llvm::omp::Clause::OMPC_is_device_ptr;
constexpr auto OMPC_lastprivate = llvm::omp::Clause::OMPC_lastprivate;
constexpr auto OMPC_linear = llvm::omp::Clause::OMPC_linear;
constexpr auto OMPC_link = llvm::omp::Clause::OMPC_link;
constexpr auto OMPC_map = llvm::omp::Clause::OMPC_map;
constexpr auto OMPC_match = llvm::omp::Clause::OMPC_match;
constexpr auto OMPC_mergeable = llvm::omp::Clause::OMPC_mergeable;
constexpr auto OMPC_nogroup = llvm::omp::Clause::OMPC_nogroup;
constexpr auto OMPC_nowait = llvm::omp::Clause::OMPC_nowait;
constexpr auto OMPC_nontemporal = llvm::omp::Clause::OMPC_nontemporal;
constexpr auto OMPC_notinbranch = llvm::omp::Clause::OMPC_notinbranch;
constexpr auto OMPC_num_tasks = llvm::omp::Clause::OMPC_num_tasks;
constexpr auto OMPC_num_teams = llvm::omp::Clause::OMPC_num_teams;
constexpr auto OMPC_num_threads = llvm::omp::Clause::OMPC_num_threads;
constexpr auto OMPC_order = llvm::omp::Clause::OMPC_order;
constexpr auto OMPC_ordered = llvm::omp::Clause::OMPC_ordered;
constexpr auto OMPC_priority = llvm::omp::Clause::OMPC_priority;
constexpr auto OMPC_private = llvm::omp::Clause::OMPC_private;
constexpr auto OMPC_proc_bind = llvm::omp::Clause::OMPC_proc_bind;
constexpr auto OMPC_read = llvm::omp::Clause::OMPC_read;
constexpr auto OMPC_reduction = llvm::omp::Clause::OMPC_reduction;
constexpr auto OMPC_relaxed = llvm::omp::Clause::OMPC_relaxed;
constexpr auto OMPC_release = llvm::omp::Clause::OMPC_release;
constexpr auto OMPC_reverse_offload = llvm::omp::Clause::OMPC_reverse_offload;
constexpr auto OMPC_safelen = llvm::omp::Clause::OMPC_safelen;
constexpr auto OMPC_schedule = llvm::omp::Clause::OMPC_schedule;
constexpr auto OMPC_seq_cst = llvm::omp::Clause::OMPC_seq_cst;
constexpr auto OMPC_shared = llvm::omp::Clause::OMPC_shared;
constexpr auto OMPC_simd = llvm::omp::Clause::OMPC_simd;
constexpr auto OMPC_simdlen = llvm::omp::Clause::OMPC_simdlen;
constexpr auto OMPC_task_reduction = llvm::omp::Clause::OMPC_task_reduction;
constexpr auto OMPC_thread_limit = llvm::omp::Clause::OMPC_thread_limit;
constexpr auto OMPC_threadprivate = llvm::omp::Clause::OMPC_threadprivate;
constexpr auto OMPC_threads = llvm::omp::Clause::OMPC_threads;
constexpr auto OMPC_to = llvm::omp::Clause::OMPC_to;
constexpr auto OMPC_unified_address = llvm::omp::Clause::OMPC_unified_address;
constexpr auto OMPC_unified_shared_memory = llvm::omp::Clause::OMPC_unified_shared_memory;
constexpr auto OMPC_uniform = llvm::omp::Clause::OMPC_uniform;
constexpr auto OMPC_unknown = llvm::omp::Clause::OMPC_unknown;
constexpr auto OMPC_untied = llvm::omp::Clause::OMPC_untied;
constexpr auto OMPC_update = llvm::omp::Clause::OMPC_update;
constexpr auto OMPC_use_device_addr = llvm::omp::Clause::OMPC_use_device_addr;
constexpr auto OMPC_use_device_ptr = llvm::omp::Clause::OMPC_use_device_ptr;
constexpr auto OMPC_uses_allocators = llvm::omp::Clause::OMPC_uses_allocators;
constexpr auto OMPC_write = llvm::omp::Clause::OMPC_write;

enum class OrderKind {
  OMP_ORDER_concurrent=2,
};

constexpr auto OMP_ORDER_concurrent = llvm::omp::OrderKind::OMP_ORDER_concurrent;

enum class ProcBindKind {
  OMP_PROC_BIND_master=2,
  OMP_PROC_BIND_close=3,
  OMP_PROC_BIND_spread=4,
  OMP_PROC_BIND_default=5,
  OMP_PROC_BIND_unknown=6,
};

constexpr auto OMP_PROC_BIND_master = llvm::omp::ProcBindKind::OMP_PROC_BIND_master;
constexpr auto OMP_PROC_BIND_close = llvm::omp::ProcBindKind::OMP_PROC_BIND_close;
constexpr auto OMP_PROC_BIND_spread = llvm::omp::ProcBindKind::OMP_PROC_BIND_spread;
constexpr auto OMP_PROC_BIND_default = llvm::omp::ProcBindKind::OMP_PROC_BIND_default;
constexpr auto OMP_PROC_BIND_unknown = llvm::omp::ProcBindKind::OMP_PROC_BIND_unknown;

enum class ScheduleKind {
  OMP_SCHEDULE_Static=2,
  OMP_SCHEDULE_Dynamic=3,
  OMP_SCHEDULE_Guided=4,
  OMP_SCHEDULE_Auto=5,
  OMP_SCHEDULE_Runtime=6,
  OMP_SCHEDULE_Default=7,
};

constexpr auto OMP_SCHEDULE_Static = llvm::omp::ScheduleKind::OMP_SCHEDULE_Static;
constexpr auto OMP_SCHEDULE_Dynamic = llvm::omp::ScheduleKind::OMP_SCHEDULE_Dynamic;
constexpr auto OMP_SCHEDULE_Guided = llvm::omp::ScheduleKind::OMP_SCHEDULE_Guided;
constexpr auto OMP_SCHEDULE_Auto = llvm::omp::ScheduleKind::OMP_SCHEDULE_Auto;
constexpr auto OMP_SCHEDULE_Runtime = llvm::omp::ScheduleKind::OMP_SCHEDULE_Runtime;
constexpr auto OMP_SCHEDULE_Default = llvm::omp::ScheduleKind::OMP_SCHEDULE_Default;

// Enumeration helper functions
Directive getOpenMPDirectiveKind(llvm::StringRef Str);

llvm::StringRef getOpenMPDirectiveName(Directive D);

Clause getOpenMPClauseKind(llvm::StringRef Str);

llvm::StringRef getOpenMPClauseName(Clause C);

/// Return true if \p C is a valid clause for \p D in version \p Version.
bool isAllowedClauseForDirective(Directive D, Clause C, unsigned Version);

OrderKind getOrderKind(StringRef);
llvm::StringRef getOpenMPOrderKindName(OrderKind);
ProcBindKind getProcBindKind(StringRef);
llvm::StringRef getOpenMPProcBindKindName(ProcBindKind);
ScheduleKind getScheduleKind(StringRef);
llvm::StringRef getOpenMPScheduleKindName(ScheduleKind);

} // namespace omp
} // namespace llvm
#endif // LLVM_OpenMP_INC
