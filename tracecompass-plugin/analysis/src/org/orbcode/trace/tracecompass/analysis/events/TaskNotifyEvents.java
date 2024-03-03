package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class TaskNotifyEvents {
	public static final int TASK_NOTIFY_STATE_PENDING = 20;
	public static final int TASK_NOTIFY_STATE_RECEIVED = 21;

	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public TaskNotifyEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("TaskNotify")
	public void taskNotified(@Field("task.name") String taskName, @Field("index") int index,
			@Field("updated_value") int value) {
		setTaskNotify(fCurrentEvent, taskName, index, value);
		setTaskNotifyState(fCurrentEvent, taskName, index, TASK_NOTIFY_STATE_PENDING);
	}

	@EventHandler("TaskNotifyReceived")
	public void taskNotifyReceived(@Field("task.name") String taskName, @Field("index") int index,
			@Field("updated_value") int value) {
		setTaskNotify(fCurrentEvent, taskName, index, value);
		setTaskNotifyState(fCurrentEvent, taskName, index, TASK_NOTIFY_STATE_RECEIVED);
	}

	private void setTaskNotify(ITmfEvent event, String taskName, int index, int value) {
		int quarkTaskNotify = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "TaskNotify",
				Integer.toString(index));
		int quarkValue = fStateSystem.getQuarkRelativeAndAdd(quarkTaskNotify, "Value");

		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), value, quarkValue);
	}

	private void setTaskNotifyState(ITmfEvent event, String taskName, int index, int state) {
		int quarkTaskNotify = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "TaskNotify",
				Integer.toString(index));
		int quarkState = fStateSystem.getQuarkRelativeAndAdd(quarkTaskNotify, "State");

		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quarkState);
	}

}
