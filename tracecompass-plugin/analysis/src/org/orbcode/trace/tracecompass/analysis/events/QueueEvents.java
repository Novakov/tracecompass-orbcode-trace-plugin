package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class QueueEvents {
	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public QueueEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("QueueCreated")
	public void queueCreated(@Field("queue.name") String queueName, @Field("capacity") int capacity) {
		setQueueCapacity(fCurrentEvent, queueName, capacity);
		setQueueCount(fCurrentEvent, queueName, 0);
	}

	@EventHandler("QueuePushed")
	public void queuePushed(@Field("queue.name") String queueName, @Field("updated_count") int newCount) {
		setQueueCount(fCurrentEvent, queueName, newCount);
	}

	@EventHandler("QueuePopped")
	public void queuePopped(@Field("queue.name") String queueName, @Field("updated_count") int newCount) {
		setQueueCount(fCurrentEvent, queueName, newCount);
	}
	
	private void setQueueCapacity(ITmfEvent event, String queue, int capacity) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Queue", queue, "Capacity");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), capacity, quark);
	}
	
	private void setQueueCount(ITmfEvent event, String queue, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Queue", queue, "Count");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}
}
