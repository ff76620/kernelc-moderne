class PriorityScheduler {
private:
    struct TaskQueue {
        List tasks;
        uint32_t quantum;
        uint8_t priority;
    };

    TaskQueue priority_queues[8];
    Task* current_task;
    uint64_t ticks;

public:
    void schedule() {
        update_priorities();
        Task* next = select_next_task();
        if (next != current_task) {
            context_switch(current_task, next);
        }
    }

    void add_task(Task* task, uint8_t priority) {
        priority = clamp(priority, 0, 7);
        priority_queues[priority].tasks.push_back(task);
        task->set_quantum(priority_queues[priority].quantum);
    }

    void yield() {
        if (current_task) {
            auto priority = current_task->get_priority();
            priority_queues[priority].tasks.push_back(current_task);
            schedule();
        }
    }
};