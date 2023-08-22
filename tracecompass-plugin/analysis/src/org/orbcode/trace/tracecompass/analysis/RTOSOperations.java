package org.orbcode.trace.tracecompass.analysis;

import org.eclipse.core.runtime.Platform;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;

public class RTOSOperations {
	private StateSystemAccessor fStateSystem;
	private ITmfEvent fCurrentEvent;

	public static final int TASK_STATE_CREATED = 0;
	public static final int TASK_STATE_RUNNING = 1;
	public static final int TASK_STATE_BLOCKED = 2;
	public static final int TASK_STATE_READY = 4;
	public static final int TASK_STATE_DELAYED = 5;

	public static final int LOCK_STATE_CREATED = 0;
	public static final int LOCK_STATE_LOCKED = 10;
	public static final int LOCK_STATE_UNLOCKED = 11;

	public RTOSOperations(StateSystemAccessor stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	public void taskCreated(String taskName) {
		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_CREATED);
	}

	public void taskSwitchedIn(String taskName) {
		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_RUNNING);
		fStateSystem.setCurrentTask(fCurrentEvent, taskName);
		modifyReadyTaskCount(-1);
	}

	public void taskSwitchedOutDelayed(String taskName) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);
		
		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_DELAYED);
	}

	public void taskSwitchedOutReady(String taskName) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);

		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);

		modifyReadyTaskCount(1);
		int preemptedCount = fStateSystem.getTaskPreemptedCount(taskName);
		preemptedCount++;
		fStateSystem.setTaskPreemptedCount(fCurrentEvent, taskName, preemptedCount);
	}

	public void taskSwitchedOutBlocked(String taskName, String objectType, String address, String operation) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);

		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_BLOCKED);
		fStateSystem.setTaskBlockedOn(fCurrentEvent, taskName, objectType, address, operation);

	}

	public void taskReadied(String taskName) {
		Integer currentState = fStateSystem.getTaskState(taskName);

		if (currentState != null && currentState != TASK_STATE_READY && currentState != TASK_STATE_RUNNING) {
			modifyReadyTaskCount(1);
		}
		if (currentState != null && currentState != TASK_STATE_RUNNING) {
			fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);
		}

		if (currentState != null && currentState == TASK_STATE_BLOCKED) {
			fStateSystem.setTaskBlockedOn(fCurrentEvent, taskName, null, null, null);
		}
	}

	public void queueCreated(String queue, int capacity) {
		fStateSystem.setQueueCapacity(fCurrentEvent, queue, capacity);
		fStateSystem.setQueueCount(fCurrentEvent, queue, 0);
	}

	public void queuePushed(String queue, int newCount) {
		fStateSystem.setQueueCount(fCurrentEvent, queue, newCount);
	}

	public void queuePopped(String queue, int newCount) {
		fStateSystem.setQueueCount(fCurrentEvent, queue, newCount);
	}

	public void binarySemaphoreCreated(String semaphore) {
		fStateSystem.setBinarySemaphoreState(fCurrentEvent, semaphore, LOCK_STATE_CREATED);
		fStateSystem.setBinarySemaphoreLockedBy(fCurrentEvent, semaphore, null);
	}

	public void binarySemaphoreLocked(String semaphore, String lockedBy) {
		fStateSystem.setBinarySemaphoreState(fCurrentEvent, semaphore, LOCK_STATE_LOCKED);
		fStateSystem.setBinarySemaphoreLockedBy(fCurrentEvent, semaphore, lockedBy);
	}

	public void binarySemaphoreUnlocked(String semaphore) {
		fStateSystem.setBinarySemaphoreState(fCurrentEvent, semaphore, LOCK_STATE_UNLOCKED);
		fStateSystem.setBinarySemaphoreLockedBy(fCurrentEvent, semaphore, null);
	}

	public void mutexCreated(String mutex) {
		fStateSystem.setMutexState(fCurrentEvent, mutex, LOCK_STATE_CREATED);
		fStateSystem.setMutexLockedBy(fCurrentEvent, mutex, null);
	}

	public void mutexLocked(String mutex, String lockedBy) {
		fStateSystem.setMutexState(fCurrentEvent, mutex, LOCK_STATE_LOCKED);
		fStateSystem.setMutexLockedBy(fCurrentEvent, mutex, lockedBy);
	}

	public void mutexUnlocked(String mutex) {
		fStateSystem.setMutexState(fCurrentEvent, mutex, LOCK_STATE_UNLOCKED);
		fStateSystem.setMutexLockedBy(fCurrentEvent, mutex, null);
	}

	private void modifyReadyTaskCount(int delta) {
		int readyCount = fStateSystem.getReadyTaskCount();
		readyCount += delta;
		fStateSystem.setReadyTaskCount(fCurrentEvent, readyCount);
	}
}
