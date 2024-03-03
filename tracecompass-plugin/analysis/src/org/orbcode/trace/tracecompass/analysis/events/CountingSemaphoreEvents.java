package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class CountingSemaphoreEvents {

	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public CountingSemaphoreEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent event) {
		fStateSystem = stateSystem;
		fCurrentEvent = event;
	}
	
	@EventHandler("CountingSemaphoreCreated")
	public void countingSemaphoreCreated(@Field("semaphore.name") String semaphore, @Field("max_count") int maxCount, @Field("initial_count") int initialCount) {
		setCountingSemaphoreCapacity(fCurrentEvent, semaphore, maxCount);
		setCountingSemaphoreCount(fCurrentEvent, semaphore, initialCount);
	}

	@EventHandler("CountingSemaphoreGiven")
	public void countingSemaphoreGiven(@Field("semaphore.name") String semaphore, @Field("updated_count") int updatedCount) {
		setCountingSemaphoreCount(fCurrentEvent, semaphore, updatedCount);
	}

	@EventHandler("CountingSemaphoreTaken")
	public void countingSemaphoreTaken(@Field("semaphore.name") String semaphore, @Field("updated_count") int updatedCount) {
		setCountingSemaphoreCount(fCurrentEvent, semaphore, updatedCount);
	}
	
	private void setCountingSemaphoreCapacity(ITmfEvent event, String semaphore, int maxCount) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CountingSemaphore", semaphore, "Capacity");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), maxCount, quark);
	}

	private void setCountingSemaphoreCount(ITmfEvent event, String semaphore, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CountingSemaphore", semaphore, "Count");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}
}
