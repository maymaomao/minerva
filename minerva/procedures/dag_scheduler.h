#pragma once
#include "procedures/dag_procedure.h"
#include "procedures/runtime_info_map.h"
#include "procedures/device_listener.h"
#include "dag/dag.h"
#include "dag/physical_dag.h"
#include "common/common.h"
#include "common/concurrent_blocking_queue.h"
#include <unordered_set>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace minerva {

class DagScheduler : public DagProcedure<PhysicalDag>, public DagMonitor<PhysicalDag>, public DeviceListener {
 public:
  DagScheduler(PhysicalDag*);
  // Wait for evaluation to finish
  void WaitForFinish();
  void GCNodes();
  // Monitor external reference changes
  void OnIncrExternRC(PhysicalDataNode*, int);
  // DAG monitor
  void OnCreateNode(DagNode*);
  void OnDeleteNode(DagNode*);
  void OnCreateEdge(DagNode*, DagNode*);
  // Device listener
  void OnOperationComplete(uint64_t);  // Synchronized
  // DAG procedure
  void Process(const std::vector<uint64_t>&);  // Synchronized

 private:
  int CalcTotalReferenceCount(PhysicalDataNode*);
  void FreeDataNodeRes(PhysicalDataNode*);
  std::unordered_set<uint64_t> FindStartFrontier(const std::vector<uint64_t>&);
  // Runtime information
  RuntimeInfoMap rt_info_;
  // Scheduler dispatcher
  ConcurrentBlockingQueue<uint64_t> dispatcher_queue_;
  void DispatcherRoutine();
  std::thread dispatcher_;
  // Evaluation finishing signal
  int num_nodes_yet_to_finish_;
  std::mutex finish_mutex_;
  std::condition_variable finish_cond_;
  DISALLOW_COPY_AND_ASSIGN(DagScheduler);
};

}  // namespace minerva

