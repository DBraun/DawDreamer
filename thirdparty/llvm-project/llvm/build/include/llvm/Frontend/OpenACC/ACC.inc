#ifdef GEN_FLANG_DIRECTIVE_CLAUSE_SETS
#undef GEN_FLANG_DIRECTIVE_CLAUSE_SETS

namespace llvm {
namespace acc {

  // Sets for atomic

  static AccClauseSet allowedClauses_ACCD_atomic {
  };

  static AccClauseSet allowedOnceClauses_ACCD_atomic {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_atomic {
  };

  static AccClauseSet requiredClauses_ACCD_atomic {
  };

  // Sets for cache

  static AccClauseSet allowedClauses_ACCD_cache {
  };

  static AccClauseSet allowedOnceClauses_ACCD_cache {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_cache {
  };

  static AccClauseSet requiredClauses_ACCD_cache {
  };

  // Sets for data

  static AccClauseSet allowedClauses_ACCD_data {
  };

  static AccClauseSet allowedOnceClauses_ACCD_data {
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_default,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_data {
  };

  static AccClauseSet requiredClauses_ACCD_data {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
  };

  // Sets for declare

  static AccClauseSet allowedClauses_ACCD_declare {
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_device_resident,
    llvm::acc::Clause::ACCC_link,
  };

  static AccClauseSet allowedOnceClauses_ACCD_declare {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_declare {
  };

  static AccClauseSet requiredClauses_ACCD_declare {
  };

  // Sets for enter data

  static AccClauseSet allowedClauses_ACCD_enter_data {
  };

  static AccClauseSet allowedOnceClauses_ACCD_enter_data {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_enter_data {
  };

  static AccClauseSet requiredClauses_ACCD_enter_data {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_copyin,
  };

  // Sets for exit data

  static AccClauseSet allowedClauses_ACCD_exit_data {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_wait,
    llvm::acc::Clause::ACCC_finalize,
  };

  static AccClauseSet allowedOnceClauses_ACCD_exit_data {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_exit_data {
  };

  static AccClauseSet requiredClauses_ACCD_exit_data {
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_delete,
    llvm::acc::Clause::ACCC_detach,
  };

  // Sets for host_data

  static AccClauseSet allowedClauses_ACCD_host_data {
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_if_present,
  };

  static AccClauseSet allowedOnceClauses_ACCD_host_data {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_host_data {
  };

  static AccClauseSet requiredClauses_ACCD_host_data {
    llvm::acc::Clause::ACCC_use_device,
  };

  // Sets for init

  static AccClauseSet allowedClauses_ACCD_init {
  };

  static AccClauseSet allowedOnceClauses_ACCD_init {
    llvm::acc::Clause::ACCC_device_num,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_if,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_init {
  };

  static AccClauseSet requiredClauses_ACCD_init {
  };

  // Sets for kernels

  static AccClauseSet allowedClauses_ACCD_kernels {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_kernels {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_num_gangs,
    llvm::acc::Clause::ACCC_num_workers,
    llvm::acc::Clause::ACCC_self,
    llvm::acc::Clause::ACCC_vector_length,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_kernels {
  };

  static AccClauseSet requiredClauses_ACCD_kernels {
  };

  // Sets for kernels loop

  static AccClauseSet allowedClauses_ACCD_kernels_loop {
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_private,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_attach,
  };

  static AccClauseSet allowedOnceClauses_ACCD_kernels_loop {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_collapse,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_gang,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_num_gangs,
    llvm::acc::Clause::ACCC_num_workers,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_self,
    llvm::acc::Clause::ACCC_tile,
    llvm::acc::Clause::ACCC_vector,
    llvm::acc::Clause::ACCC_vector_length,
    llvm::acc::Clause::ACCC_wait,
    llvm::acc::Clause::ACCC_worker,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_kernels_loop {
    llvm::acc::Clause::ACCC_auto,
    llvm::acc::Clause::ACCC_independent,
    llvm::acc::Clause::ACCC_seq,
  };

  static AccClauseSet requiredClauses_ACCD_kernels_loop {
  };

  // Sets for loop

  static AccClauseSet allowedClauses_ACCD_loop {
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_private,
  };

  static AccClauseSet allowedOnceClauses_ACCD_loop {
    llvm::acc::Clause::ACCC_collapse,
    llvm::acc::Clause::ACCC_gang,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_tile,
    llvm::acc::Clause::ACCC_vector,
    llvm::acc::Clause::ACCC_worker,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_loop {
    llvm::acc::Clause::ACCC_auto,
    llvm::acc::Clause::ACCC_independent,
    llvm::acc::Clause::ACCC_seq,
  };

  static AccClauseSet requiredClauses_ACCD_loop {
  };

  // Sets for parallel

  static AccClauseSet allowedClauses_ACCD_parallel {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_private,
    llvm::acc::Clause::ACCC_firstprivate,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_parallel {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_num_gangs,
    llvm::acc::Clause::ACCC_num_workers,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_self,
    llvm::acc::Clause::ACCC_vector_length,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_parallel {
  };

  static AccClauseSet requiredClauses_ACCD_parallel {
  };

  // Sets for parallel loop

  static AccClauseSet allowedClauses_ACCD_parallel_loop {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_firstprivate,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_private,
    llvm::acc::Clause::ACCC_tile,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_parallel_loop {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_collapse,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_gang,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_num_gangs,
    llvm::acc::Clause::ACCC_num_workers,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_self,
    llvm::acc::Clause::ACCC_vector,
    llvm::acc::Clause::ACCC_vector_length,
    llvm::acc::Clause::ACCC_worker,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_parallel_loop {
    llvm::acc::Clause::ACCC_auto,
    llvm::acc::Clause::ACCC_independent,
    llvm::acc::Clause::ACCC_seq,
  };

  static AccClauseSet requiredClauses_ACCD_parallel_loop {
  };

  // Sets for routine

  static AccClauseSet allowedClauses_ACCD_routine {
  };

  static AccClauseSet allowedOnceClauses_ACCD_routine {
    llvm::acc::Clause::ACCC_bind,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_nohost,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_routine {
  };

  static AccClauseSet requiredClauses_ACCD_routine {
    llvm::acc::Clause::ACCC_gang,
    llvm::acc::Clause::ACCC_seq,
    llvm::acc::Clause::ACCC_vector,
    llvm::acc::Clause::ACCC_worker,
  };

  // Sets for serial

  static AccClauseSet allowedClauses_ACCD_serial {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_private,
    llvm::acc::Clause::ACCC_firstprivate,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_serial {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_self,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_serial {
  };

  static AccClauseSet requiredClauses_ACCD_serial {
  };

  // Sets for serial loop

  static AccClauseSet allowedClauses_ACCD_serial_loop {
    llvm::acc::Clause::ACCC_attach,
    llvm::acc::Clause::ACCC_copy,
    llvm::acc::Clause::ACCC_copyin,
    llvm::acc::Clause::ACCC_copyout,
    llvm::acc::Clause::ACCC_create,
    llvm::acc::Clause::ACCC_deviceptr,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_firstprivate,
    llvm::acc::Clause::ACCC_no_create,
    llvm::acc::Clause::ACCC_present,
    llvm::acc::Clause::ACCC_private,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_serial_loop {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_collapse,
    llvm::acc::Clause::ACCC_default,
    llvm::acc::Clause::ACCC_gang,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_reduction,
    llvm::acc::Clause::ACCC_self,
    llvm::acc::Clause::ACCC_tile,
    llvm::acc::Clause::ACCC_vector,
    llvm::acc::Clause::ACCC_worker,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_serial_loop {
    llvm::acc::Clause::ACCC_auto,
    llvm::acc::Clause::ACCC_independent,
    llvm::acc::Clause::ACCC_seq,
  };

  static AccClauseSet requiredClauses_ACCD_serial_loop {
  };

  // Sets for set

  static AccClauseSet allowedClauses_ACCD_set {
  };

  static AccClauseSet allowedOnceClauses_ACCD_set {
    llvm::acc::Clause::ACCC_default_async,
    llvm::acc::Clause::ACCC_device_num,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_if,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_set {
  };

  static AccClauseSet requiredClauses_ACCD_set {
    llvm::acc::Clause::ACCC_default_async,
    llvm::acc::Clause::ACCC_device_num,
    llvm::acc::Clause::ACCC_device_type,
  };

  // Sets for shutdown

  static AccClauseSet allowedClauses_ACCD_shutdown {
  };

  static AccClauseSet allowedOnceClauses_ACCD_shutdown {
    llvm::acc::Clause::ACCC_device_num,
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_if,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_shutdown {
  };

  static AccClauseSet requiredClauses_ACCD_shutdown {
  };

  // Sets for unknown

  static AccClauseSet allowedClauses_ACCD_unknown {
  };

  static AccClauseSet allowedOnceClauses_ACCD_unknown {
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_unknown {
  };

  static AccClauseSet requiredClauses_ACCD_unknown {
  };

  // Sets for update

  static AccClauseSet allowedClauses_ACCD_update {
    llvm::acc::Clause::ACCC_device_type,
    llvm::acc::Clause::ACCC_wait,
  };

  static AccClauseSet allowedOnceClauses_ACCD_update {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_if,
    llvm::acc::Clause::ACCC_if_present,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_update {
  };

  static AccClauseSet requiredClauses_ACCD_update {
    llvm::acc::Clause::ACCC_device,
    llvm::acc::Clause::ACCC_host,
    llvm::acc::Clause::ACCC_self,
  };

  // Sets for wait

  static AccClauseSet allowedClauses_ACCD_wait {
  };

  static AccClauseSet allowedOnceClauses_ACCD_wait {
    llvm::acc::Clause::ACCC_async,
    llvm::acc::Clause::ACCC_if,
  };

  static AccClauseSet allowedExclusiveClauses_ACCD_wait {
  };

  static AccClauseSet requiredClauses_ACCD_wait {
  };
} // namespace acc
} // namespace llvm

#endif // GEN_FLANG_DIRECTIVE_CLAUSE_SETS

#ifdef GEN_FLANG_DIRECTIVE_CLAUSE_MAP
#undef GEN_FLANG_DIRECTIVE_CLAUSE_MAP

{
  {llvm::acc::Directive::ACCD_atomic,
    {
      llvm::acc::allowedClauses_ACCD_atomic,
      llvm::acc::allowedOnceClauses_ACCD_atomic,
      llvm::acc::allowedExclusiveClauses_ACCD_atomic,
      llvm::acc::requiredClauses_ACCD_atomic,
    }
  },
  {llvm::acc::Directive::ACCD_cache,
    {
      llvm::acc::allowedClauses_ACCD_cache,
      llvm::acc::allowedOnceClauses_ACCD_cache,
      llvm::acc::allowedExclusiveClauses_ACCD_cache,
      llvm::acc::requiredClauses_ACCD_cache,
    }
  },
  {llvm::acc::Directive::ACCD_data,
    {
      llvm::acc::allowedClauses_ACCD_data,
      llvm::acc::allowedOnceClauses_ACCD_data,
      llvm::acc::allowedExclusiveClauses_ACCD_data,
      llvm::acc::requiredClauses_ACCD_data,
    }
  },
  {llvm::acc::Directive::ACCD_declare,
    {
      llvm::acc::allowedClauses_ACCD_declare,
      llvm::acc::allowedOnceClauses_ACCD_declare,
      llvm::acc::allowedExclusiveClauses_ACCD_declare,
      llvm::acc::requiredClauses_ACCD_declare,
    }
  },
  {llvm::acc::Directive::ACCD_enter_data,
    {
      llvm::acc::allowedClauses_ACCD_enter_data,
      llvm::acc::allowedOnceClauses_ACCD_enter_data,
      llvm::acc::allowedExclusiveClauses_ACCD_enter_data,
      llvm::acc::requiredClauses_ACCD_enter_data,
    }
  },
  {llvm::acc::Directive::ACCD_exit_data,
    {
      llvm::acc::allowedClauses_ACCD_exit_data,
      llvm::acc::allowedOnceClauses_ACCD_exit_data,
      llvm::acc::allowedExclusiveClauses_ACCD_exit_data,
      llvm::acc::requiredClauses_ACCD_exit_data,
    }
  },
  {llvm::acc::Directive::ACCD_host_data,
    {
      llvm::acc::allowedClauses_ACCD_host_data,
      llvm::acc::allowedOnceClauses_ACCD_host_data,
      llvm::acc::allowedExclusiveClauses_ACCD_host_data,
      llvm::acc::requiredClauses_ACCD_host_data,
    }
  },
  {llvm::acc::Directive::ACCD_init,
    {
      llvm::acc::allowedClauses_ACCD_init,
      llvm::acc::allowedOnceClauses_ACCD_init,
      llvm::acc::allowedExclusiveClauses_ACCD_init,
      llvm::acc::requiredClauses_ACCD_init,
    }
  },
  {llvm::acc::Directive::ACCD_kernels,
    {
      llvm::acc::allowedClauses_ACCD_kernels,
      llvm::acc::allowedOnceClauses_ACCD_kernels,
      llvm::acc::allowedExclusiveClauses_ACCD_kernels,
      llvm::acc::requiredClauses_ACCD_kernels,
    }
  },
  {llvm::acc::Directive::ACCD_kernels_loop,
    {
      llvm::acc::allowedClauses_ACCD_kernels_loop,
      llvm::acc::allowedOnceClauses_ACCD_kernels_loop,
      llvm::acc::allowedExclusiveClauses_ACCD_kernels_loop,
      llvm::acc::requiredClauses_ACCD_kernels_loop,
    }
  },
  {llvm::acc::Directive::ACCD_loop,
    {
      llvm::acc::allowedClauses_ACCD_loop,
      llvm::acc::allowedOnceClauses_ACCD_loop,
      llvm::acc::allowedExclusiveClauses_ACCD_loop,
      llvm::acc::requiredClauses_ACCD_loop,
    }
  },
  {llvm::acc::Directive::ACCD_parallel,
    {
      llvm::acc::allowedClauses_ACCD_parallel,
      llvm::acc::allowedOnceClauses_ACCD_parallel,
      llvm::acc::allowedExclusiveClauses_ACCD_parallel,
      llvm::acc::requiredClauses_ACCD_parallel,
    }
  },
  {llvm::acc::Directive::ACCD_parallel_loop,
    {
      llvm::acc::allowedClauses_ACCD_parallel_loop,
      llvm::acc::allowedOnceClauses_ACCD_parallel_loop,
      llvm::acc::allowedExclusiveClauses_ACCD_parallel_loop,
      llvm::acc::requiredClauses_ACCD_parallel_loop,
    }
  },
  {llvm::acc::Directive::ACCD_routine,
    {
      llvm::acc::allowedClauses_ACCD_routine,
      llvm::acc::allowedOnceClauses_ACCD_routine,
      llvm::acc::allowedExclusiveClauses_ACCD_routine,
      llvm::acc::requiredClauses_ACCD_routine,
    }
  },
  {llvm::acc::Directive::ACCD_serial,
    {
      llvm::acc::allowedClauses_ACCD_serial,
      llvm::acc::allowedOnceClauses_ACCD_serial,
      llvm::acc::allowedExclusiveClauses_ACCD_serial,
      llvm::acc::requiredClauses_ACCD_serial,
    }
  },
  {llvm::acc::Directive::ACCD_serial_loop,
    {
      llvm::acc::allowedClauses_ACCD_serial_loop,
      llvm::acc::allowedOnceClauses_ACCD_serial_loop,
      llvm::acc::allowedExclusiveClauses_ACCD_serial_loop,
      llvm::acc::requiredClauses_ACCD_serial_loop,
    }
  },
  {llvm::acc::Directive::ACCD_set,
    {
      llvm::acc::allowedClauses_ACCD_set,
      llvm::acc::allowedOnceClauses_ACCD_set,
      llvm::acc::allowedExclusiveClauses_ACCD_set,
      llvm::acc::requiredClauses_ACCD_set,
    }
  },
  {llvm::acc::Directive::ACCD_shutdown,
    {
      llvm::acc::allowedClauses_ACCD_shutdown,
      llvm::acc::allowedOnceClauses_ACCD_shutdown,
      llvm::acc::allowedExclusiveClauses_ACCD_shutdown,
      llvm::acc::requiredClauses_ACCD_shutdown,
    }
  },
  {llvm::acc::Directive::ACCD_unknown,
    {
      llvm::acc::allowedClauses_ACCD_unknown,
      llvm::acc::allowedOnceClauses_ACCD_unknown,
      llvm::acc::allowedExclusiveClauses_ACCD_unknown,
      llvm::acc::requiredClauses_ACCD_unknown,
    }
  },
  {llvm::acc::Directive::ACCD_update,
    {
      llvm::acc::allowedClauses_ACCD_update,
      llvm::acc::allowedOnceClauses_ACCD_update,
      llvm::acc::allowedExclusiveClauses_ACCD_update,
      llvm::acc::requiredClauses_ACCD_update,
    }
  },
  {llvm::acc::Directive::ACCD_wait,
    {
      llvm::acc::allowedClauses_ACCD_wait,
      llvm::acc::allowedOnceClauses_ACCD_wait,
      llvm::acc::allowedExclusiveClauses_ACCD_wait,
      llvm::acc::requiredClauses_ACCD_wait,
    }
  },
}

#endif // GEN_FLANG_DIRECTIVE_CLAUSE_MAP

#ifdef GEN_FLANG_CLAUSE_PARSER_CLASSES
#undef GEN_FLANG_CLAUSE_PARSER_CLASSES

WRAPPER_CLASS(Async, std::optional<ScalarIntExpr>);
WRAPPER_CLASS(Attach, AccObjectList);
EMPTY_CLASS(Auto);
WRAPPER_CLASS(Bind, AccBindClause);
EMPTY_CLASS(Capture);
WRAPPER_CLASS(Collapse, ScalarIntConstantExpr);
WRAPPER_CLASS(Copy, AccObjectList);
WRAPPER_CLASS(Copyin, AccObjectListWithModifier);
WRAPPER_CLASS(Copyout, AccObjectListWithModifier);
WRAPPER_CLASS(Create, AccObjectListWithModifier);
WRAPPER_CLASS(Default, AccDefaultClause);
WRAPPER_CLASS(DefaultAsync, ScalarIntExpr);
WRAPPER_CLASS(Delete, AccObjectList);
WRAPPER_CLASS(Detach, AccObjectList);
WRAPPER_CLASS(Device, AccObjectList);
WRAPPER_CLASS(DeviceNum, ScalarIntExpr);
WRAPPER_CLASS(Deviceptr, AccObjectList);
WRAPPER_CLASS(DeviceResident, AccObjectList);
WRAPPER_CLASS(DeviceType, std::optional<std::list<ScalarIntExpr>>);
EMPTY_CLASS(Finalize);
WRAPPER_CLASS(Firstprivate, AccObjectList);
WRAPPER_CLASS(Gang, std::optional<AccGangArgument>);
WRAPPER_CLASS(Host, AccObjectList);
WRAPPER_CLASS(If, ScalarLogicalExpr);
EMPTY_CLASS(IfPresent);
EMPTY_CLASS(Independent);
WRAPPER_CLASS(Link, AccObjectList);
WRAPPER_CLASS(NoCreate, AccObjectList);
EMPTY_CLASS(Nohost);
WRAPPER_CLASS(NumGangs, ScalarIntExpr);
WRAPPER_CLASS(NumWorkers, ScalarIntExpr);
WRAPPER_CLASS(Present, AccObjectList);
WRAPPER_CLASS(Private, AccObjectList);
EMPTY_CLASS(Read);
WRAPPER_CLASS(Reduction, AccObjectListWithReduction);
WRAPPER_CLASS(Self, AccSelfClause);
EMPTY_CLASS(Seq);
WRAPPER_CLASS(Tile, AccTileExprList);
EMPTY_CLASS(Unknown);
WRAPPER_CLASS(UseDevice, AccObjectList);
WRAPPER_CLASS(Vector, std::optional<ScalarIntExpr>);
WRAPPER_CLASS(VectorLength, ScalarIntExpr);
WRAPPER_CLASS(Wait, std::optional<AccWaitArgument>);
WRAPPER_CLASS(Worker, std::optional<ScalarIntExpr>);
EMPTY_CLASS(Write);

#endif // GEN_FLANG_CLAUSE_PARSER_CLASSES

#ifdef GEN_FLANG_CLAUSE_PARSER_CLASSES_LIST
#undef GEN_FLANG_CLAUSE_PARSER_CLASSES_LIST

Async
, Attach
, Auto
, Bind
, Capture
, Collapse
, Copy
, Copyin
, Copyout
, Create
, Default
, DefaultAsync
, Delete
, Detach
, Device
, DeviceNum
, Deviceptr
, DeviceResident
, DeviceType
, Finalize
, Firstprivate
, Gang
, Host
, If
, IfPresent
, Independent
, Link
, NoCreate
, Nohost
, NumGangs
, NumWorkers
, Present
, Private
, Read
, Reduction
, Self
, Seq
, Tile
, Unknown
, UseDevice
, Vector
, VectorLength
, Wait
, Worker
, Write

#endif // GEN_FLANG_CLAUSE_PARSER_CLASSES_LIST

#ifdef GEN_FLANG_DUMP_PARSE_TREE_CLAUSES
#undef GEN_FLANG_DUMP_PARSE_TREE_CLAUSES

NODE(AccClause, Async)
NODE(AccClause, Attach)
NODE(AccClause, Auto)
NODE(AccClause, Bind)
NODE(AccClause, Capture)
NODE(AccClause, Collapse)
NODE(AccClause, Copy)
NODE(AccClause, Copyin)
NODE(AccClause, Copyout)
NODE(AccClause, Create)
NODE(AccClause, Default)
NODE(AccClause, DefaultAsync)
NODE(AccClause, Delete)
NODE(AccClause, Detach)
NODE(AccClause, Device)
NODE(AccClause, DeviceNum)
NODE(AccClause, Deviceptr)
NODE(AccClause, DeviceResident)
NODE(AccClause, DeviceType)
NODE(AccClause, Finalize)
NODE(AccClause, Firstprivate)
NODE(AccClause, Gang)
NODE(AccClause, Host)
NODE(AccClause, If)
NODE(AccClause, IfPresent)
NODE(AccClause, Independent)
NODE(AccClause, Link)
NODE(AccClause, NoCreate)
NODE(AccClause, Nohost)
NODE(AccClause, NumGangs)
NODE(AccClause, NumWorkers)
NODE(AccClause, Present)
NODE(AccClause, Private)
NODE(AccClause, Read)
NODE(AccClause, Reduction)
NODE(AccClause, Self)
NODE(AccClause, Seq)
NODE(AccClause, Tile)
NODE(AccClause, Unknown)
NODE(AccClause, UseDevice)
NODE(AccClause, Vector)
NODE(AccClause, VectorLength)
NODE(AccClause, Wait)
NODE(AccClause, Worker)
NODE(AccClause, Write)

#endif // GEN_FLANG_DUMP_PARSE_TREE_CLAUSES

#ifdef GEN_FLANG_CLAUSE_UNPARSE
#undef GEN_FLANG_CLAUSE_UNPARSE

void Unparse(const AccClause::Async &x) {
  Word("ASYNC");
  Walk("(", x.v, ")");
}
void Unparse(const AccClause::Attach &x) {
  Word("ATTACH");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Auto &) { Word("AUTO"); }
void Unparse(const AccClause::Bind &x) {
  Word("BIND");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Capture &) { Word("CAPTURE"); }
void Unparse(const AccClause::Collapse &x) {
  Word("COLLAPSE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Copy &x) {
  Word("COPY");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Copyin &x) {
  Word("COPYIN");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Copyout &x) {
  Word("COPYOUT");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Create &x) {
  Word("CREATE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Default &x) {
  Word("DEFAULT");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::DefaultAsync &x) {
  Word("DEFAULT_ASYNC");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Delete &x) {
  Word("DELETE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Detach &x) {
  Word("DETACH");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Device &x) {
  Word("DEVICE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::DeviceNum &x) {
  Word("DEVICE_NUM");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Deviceptr &x) {
  Word("DEVICEPTR");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::DeviceResident &x) {
  Word("DEVICE_RESIDENT");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::DeviceType &x) {
  Word("DEVICE_TYPE");
  Put("(");
  if (x.v.has_value())
    Walk(x.v, ",");
  else
    Put("*");
  Put(")");
}
void Before(const AccClause::Finalize &) { Word("FINALIZE"); }
void Unparse(const AccClause::Firstprivate &x) {
  Word("FIRSTPRIVATE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Gang &x) {
  Word("GANG");
  Walk("(", x.v, ")");
}
void Unparse(const AccClause::Host &x) {
  Word("HOST");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::If &x) {
  Word("IF");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::IfPresent &) { Word("IF_PRESENT"); }
void Before(const AccClause::Independent &) { Word("INDEPENDENT"); }
void Unparse(const AccClause::Link &x) {
  Word("LINK");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::NoCreate &x) {
  Word("NO_CREATE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Nohost &) { Word("NOHOST"); }
void Unparse(const AccClause::NumGangs &x) {
  Word("NUM_GANGS");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::NumWorkers &x) {
  Word("NUM_WORKERS");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Present &x) {
  Word("PRESENT");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Private &x) {
  Word("PRIVATE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Read &) { Word("READ"); }
void Unparse(const AccClause::Reduction &x) {
  Word("REDUCTION");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Self &x) {
  Word("SELF");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Seq &) { Word("SEQ"); }
void Unparse(const AccClause::Tile &x) {
  Word("TILE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Before(const AccClause::Unknown &) { Word("UNKNOWN"); }
void Unparse(const AccClause::UseDevice &x) {
  Word("USE_DEVICE");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Vector &x) {
  Word("VECTOR");
  Walk("(", x.v, ")");
}
void Unparse(const AccClause::VectorLength &x) {
  Word("VECTOR_LENGTH");
  Put("(");
  Walk(x.v);
  Put(")");
}
void Unparse(const AccClause::Wait &x) {
  Word("WAIT");
  Walk("(", x.v, ")");
}
void Unparse(const AccClause::Worker &x) {
  Word("WORKER");
  Walk("(", x.v, ")");
}
void Before(const AccClause::Write &) { Word("WRITE"); }

#endif // GEN_FLANG_CLAUSE_UNPARSE

#ifdef GEN_CLANG_CLAUSE_CLASS
#undef GEN_CLANG_CLAUSE_CLASS

#ifndef CLAUSE
#define CLAUSE(Enum, Str, Implicit)
#endif
#ifndef CLAUSE_CLASS
#define CLAUSE_CLASS(Enum, Str, Class)
#endif
#ifndef CLAUSE_NO_CLASS
#define CLAUSE_NO_CLASS(Enum, Str)
#endif

#define __CLAUSE(Name, Class)                      \
  CLAUSE(ACCC_##Name, #Name, /* Implicit */ false) \
  CLAUSE_CLASS(ACCC_##Name, #Name, Class)
#define __CLAUSE_NO_CLASS(Name)                    \
  CLAUSE(ACCC_##Name, #Name, /* Implicit */ false) \
  CLAUSE_NO_CLASS(ACCC_##Name, #Name)
#define __IMPLICIT_CLAUSE_CLASS(Name, Str, Class)  \
  CLAUSE(ACCC_##Name, Str, /* Implicit */ true)    \
  CLAUSE_CLASS(ACCC_##Name, Str, Class)
#define __IMPLICIT_CLAUSE_NO_CLASS(Name, Str)      \
  CLAUSE(ACCC_##Name, Str, /* Implicit */ true)    \
  CLAUSE_NO_CLASS(ACCC_##Name, Str)

__CLAUSE_NO_CLASS(async)
__CLAUSE_NO_CLASS(attach)
__CLAUSE_NO_CLASS(auto)
__CLAUSE_NO_CLASS(bind)
__CLAUSE_NO_CLASS(capture)
__CLAUSE_NO_CLASS(collapse)
__CLAUSE_NO_CLASS(copy)
__CLAUSE_NO_CLASS(copyin)
__CLAUSE_NO_CLASS(copyout)
__CLAUSE_NO_CLASS(create)
__CLAUSE_NO_CLASS(default)
__CLAUSE_NO_CLASS(default_async)
__CLAUSE_NO_CLASS(delete)
__CLAUSE_NO_CLASS(detach)
__CLAUSE_NO_CLASS(device)
__CLAUSE_NO_CLASS(device_num)
__CLAUSE_NO_CLASS(deviceptr)
__CLAUSE_NO_CLASS(device_resident)
__CLAUSE_NO_CLASS(device_type)
__CLAUSE_NO_CLASS(finalize)
__CLAUSE_NO_CLASS(firstprivate)
__CLAUSE_NO_CLASS(gang)
__CLAUSE_NO_CLASS(host)
__CLAUSE_NO_CLASS(if)
__CLAUSE_NO_CLASS(if_present)
__CLAUSE_NO_CLASS(independent)
__CLAUSE_NO_CLASS(link)
__CLAUSE_NO_CLASS(no_create)
__CLAUSE_NO_CLASS(nohost)
__CLAUSE_NO_CLASS(num_gangs)
__CLAUSE_NO_CLASS(num_workers)
__CLAUSE_NO_CLASS(present)
__CLAUSE_NO_CLASS(private)
__CLAUSE_NO_CLASS(read)
__CLAUSE_NO_CLASS(reduction)
__CLAUSE_NO_CLASS(self)
__CLAUSE_NO_CLASS(seq)
__CLAUSE_NO_CLASS(tile)
__CLAUSE_NO_CLASS(unknown)
__CLAUSE_NO_CLASS(use_device)
__CLAUSE_NO_CLASS(vector)
__CLAUSE_NO_CLASS(vector_length)
__CLAUSE_NO_CLASS(wait)
__CLAUSE_NO_CLASS(worker)
__CLAUSE_NO_CLASS(write)

#undef __IMPLICIT_CLAUSE_NO_CLASS
#undef __IMPLICIT_CLAUSE_CLASS
#undef __CLAUSE
#undef CLAUSE_NO_CLASS
#undef CLAUSE_CLASS
#undef CLAUSE

#endif // GEN_CLANG_CLAUSE_CLASS

