package org.orbcode.trace.tracecompass.analysis.events;

import java.util.ArrayList;
import java.util.List;

import org.eclipse.jdt.annotation.NonNull;
import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.statesystem.core.statevalue.ITmfStateValue;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class TaskEvents {
	private ITmfStateSystemBuilder fStateSystem;
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

	public TaskEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("TaskCreated")
	public void taskCreated(@Field("task.name") String taskName) {
		setTaskState(fCurrentEvent, taskName, TASK_STATE_CREATED);
	}

	@EventHandler("TaskSwitchedIn")
	public void taskSwitchedIn(@Field("task.name") String taskName) {
		setTaskState(fCurrentEvent, taskName, TASK_STATE_RUNNING);
		setCurrentTask(fCurrentEvent, taskName);
		updateReadyTaskCount();
	}

	@EventHandler("TaskSwitchedOut")
	public void taskSwitchedOut(@Field("task.name") String taskName, @Field("switch_reason") String switchReason,
			@Field("still_ready") boolean stillReady, @Field("blocked_on.type") String objectType,
			@Field("blocked_on_object.name") String blockingObjectName,
			@Field("blocked_on.operation") String blockingOperation) {
		if (stillReady) {
			taskSwitchedOutReady(taskName);
		} else if (switchReason.equals("Delayed")) {
			taskSwitchedOutDelayed(taskName);
		} else {
			taskSwitchedOutBlocked(taskName, objectType, blockingObjectName, blockingOperation);
		}
	}

	public void taskSwitchedOutDelayed(String taskName) {
		setCurrentTask(fCurrentEvent, null);

		setTaskState(fCurrentEvent, taskName, TASK_STATE_DELAYED);
		updateReadyTaskCount();
	}

	public void taskSwitchedOutReady(String taskName) {
		setCurrentTask(fCurrentEvent, null);

		setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);

		updateReadyTaskCount();
		int preemptedCount = getTaskPreemptedCount(taskName);
		preemptedCount++;
		setTaskPreemptedCount(fCurrentEvent, taskName, preemptedCount);
	}

	public void taskSwitchedOutBlocked(String taskName, String objectType, String address, String operation) {
		setCurrentTask(fCurrentEvent, null);

		setTaskState(fCurrentEvent, taskName, TASK_STATE_BLOCKED);
		setTaskBlockedOn(fCurrentEvent, taskName, objectType, address, operation);
		updateReadyTaskCount();
	}

	@EventHandler("TaskReadied")
	public void taskReadied(@Field("task.name") String taskName) {
		Integer currentState = getTaskState(taskName);

		if (currentState != null && currentState != TASK_STATE_RUNNING) {
			setTaskState(fCurrentEvent, taskName, TASK_STATE_READY);
		}

		if (currentState != null && currentState == TASK_STATE_BLOCKED) {
			setTaskBlockedOn(fCurrentEvent, taskName, null, null, null);
		}

		updateReadyTaskCount();
	}

	private void updateReadyTaskCount() {
		List<Integer> states = queryTaskStates();

		long readyCount = states.stream().filter(t -> t == TASK_STATE_READY).count();
		setReadyTaskCount(fCurrentEvent, (int) readyCount);
	}

	private void setTaskState(ITmfEvent event, String taskName, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}

	private Integer getTaskState(String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "State");
		@NonNull
		ITmfStateValue ongoing = fStateSystem.queryOngoingState(quark);
		if (ongoing.isNull()) {
			return null;
		}

		return ongoing.unboxInt();
	}

	private void setTaskPreemptedCount(ITmfEvent event, String taskName, int newCount) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "PreemptedCount");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), newCount, quark);
	}

	private int getTaskPreemptedCount(String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "PreemptedCount");
		Object ongoing = fStateSystem.queryOngoing(quark);
		if (ongoing == null) {
			return 0;
		}

		return (int) ongoing;
	}

	private void setTaskBlockedOn(ITmfEvent event, String taskName, String objectType, String address,
			String operation) {
		int quarkBlockedOn = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "BlockedOn");
		int quarkObjectType = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Type");
		int quarkAddress = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Address");
		int quarkOperation = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Operation");

		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), objectType, quarkObjectType);
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), address, quarkAddress);
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), operation, quarkOperation);

		ArrayList<String> parts = new ArrayList<String>();
		if (objectType != null) {
			parts.add(objectType);
		}
		if (address != null) {
			parts.add(address);
		}
		if (operation != null) {
			parts.add(operation);
		}

		if (parts.size() == 0) {
			fStateSystem.modifyAttribute(event.getTimestamp().getValue(), null, quarkBlockedOn);
		} else {
			String blockedOn = String.join("/", parts);
			fStateSystem.modifyAttribute(event.getTimestamp().getValue(), blockedOn, quarkBlockedOn);
		}
	}

	private void setReadyTaskCount(ITmfEvent event, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("ReadyTaskCount");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}

	private List<Integer> queryTaskStates() {
		@NonNull
		List<@NonNull Integer> quarks = fStateSystem.getQuarks("Task", "*", "State");

		List<@NonNull Integer> states = new ArrayList<Integer>();

		for (int taskStateQuark : quarks) {
			Object state = fStateSystem.queryOngoing(taskStateQuark);
			if (state == null) {
				continue;
			}

			states.add((Integer) state);
		}

		return states;
	}

	private void setCurrentTask(ITmfEvent event, String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CurrentTask");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), taskName, quark);
	}
}
