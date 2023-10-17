package org.orbcode.trace.tracecompass.analysis;

import java.util.List;

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

	public static final int TASK_NOTIFY_STATE_PENDING = 20;
	public static final int TASK_NOTIFY_STATE_RECEIVED = 21;

	public static final int EXCEPTION_STATE_ACTIVE = 100;

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
		updateReadyTaskCount();
	}

	public void taskSwitchedOutDelayed(String taskName) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);

		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_DELAYED);
		updateReadyTaskCount();
	}

	public void taskSwitchedOutReady(String taskName) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);

		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);

		updateReadyTaskCount();
		int preemptedCount = fStateSystem.getTaskPreemptedCount(taskName);
		preemptedCount++;
		fStateSystem.setTaskPreemptedCount(fCurrentEvent, taskName, preemptedCount);
	}

	public void taskSwitchedOutBlocked(String taskName, String objectType, String address, String operation) {
		fStateSystem.setCurrentTask(fCurrentEvent, null);

		fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_BLOCKED);
		fStateSystem.setTaskBlockedOn(fCurrentEvent, taskName, objectType, address, operation);
		updateReadyTaskCount();
	}

	public void taskReadied(String taskName) {
		Integer currentState = fStateSystem.getTaskState(taskName);

		if (currentState != null && currentState != TASK_STATE_RUNNING) {
			fStateSystem.setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);
		}

		if (currentState != null && currentState == TASK_STATE_BLOCKED) {
			fStateSystem.setTaskBlockedOn(fCurrentEvent, taskName, null, null, null);
		}

		updateReadyTaskCount();
	}

	public void taskNotified(String taskName, int index, int value) {
		fStateSystem.setTaskNotify(fCurrentEvent, taskName, index, value);
		fStateSystem.setTaskNotifyState(fCurrentEvent, taskName, index, TASK_NOTIFY_STATE_PENDING);
	}

	public void taskNotifyReceived(String taskName, int index, int value) {
		fStateSystem.setTaskNotify(fCurrentEvent, taskName, index, value);
		fStateSystem.setTaskNotifyState(fCurrentEvent, taskName, index, TASK_NOTIFY_STATE_RECEIVED);
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

	public void countingSemaphoreCreated(String semaphore, int maxCount, int initialCount) {
		fStateSystem.setCountingSemaphoreCapacity(fCurrentEvent, semaphore, maxCount);
		fStateSystem.setCountingSemaphoreCount(fCurrentEvent, semaphore, initialCount);
	}

	public void countingSemaphoreGiven(String semaphore, int updatedCount) {
		fStateSystem.setCountingSemaphoreCount(fCurrentEvent, semaphore, updatedCount);
	}

	public void countingSemaphoreTaken(String semaphore, int updatedCount) {
		fStateSystem.setCountingSemaphoreCount(fCurrentEvent, semaphore, updatedCount);
	}

	public void exceptionEntered(int exceptionNumber) {
		fStateSystem.setExceptionState(fCurrentEvent, exceptionNumber, EXCEPTION_STATE_ACTIVE);
		fStateSystem.setCurrentExceptionNumber(fCurrentEvent, exceptionNumber);
	}

	public void exceptionExited(int exceptionNumber) {
		fStateSystem.setExceptionState(fCurrentEvent, exceptionNumber, null);
	}

	public void exceptionReturned(int exceptionNumber) {
		fStateSystem.setCurrentExceptionNumber(fCurrentEvent, exceptionNumber);
	}

	private void updateReadyTaskCount() {
		List<Integer> states = fStateSystem.queryTaskStates();

		long readyCount = states.stream().filter(t -> t == TASK_STATE_READY).count();
		fStateSystem.setReadyTaskCount(fCurrentEvent, (int) readyCount);
	}
}
