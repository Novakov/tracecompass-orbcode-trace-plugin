package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class BinarySemaphoreEvents {
	public static final int LOCK_STATE_CREATED = 0;
	public static final int LOCK_STATE_LOCKED = 10;
	public static final int LOCK_STATE_UNLOCKED = 11;
	
	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public BinarySemaphoreEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("BinarySemaphoreCreated")
	public void binarySemaphoreCreated(@Field("semaphore.name") String semaphoreName) {
		setBinarySemaphoreState(fCurrentEvent, semaphoreName, LOCK_STATE_CREATED);
		setBinarySemaphoreLockedBy(fCurrentEvent, semaphoreName, null);
	}

	@EventHandler("BinarySemaphoreLocked")
	public void binarySemaphoreLocked(@Field("semaphore.name") String semaphoreName,
			@Field("current_task.name") String lockedBy) {
		setBinarySemaphoreState(fCurrentEvent, semaphoreName, LOCK_STATE_LOCKED);
		setBinarySemaphoreLockedBy(fCurrentEvent, semaphoreName, lockedBy);
	}

	@EventHandler("BinarySemaphoreUnlocked")
	public void binarySemaphoreUnlocked(@Field("semaphore.name") String semaphoreName) {
		setBinarySemaphoreState(fCurrentEvent, semaphoreName, LOCK_STATE_UNLOCKED);
		setBinarySemaphoreLockedBy(fCurrentEvent, semaphoreName, null);
	}
	
	private void setBinarySemaphoreState(ITmfEvent event, String semaphore, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("BinarySemaphore", semaphore, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}
	
	private void setBinarySemaphoreLockedBy(ITmfEvent event, String semaphore, String lockedBy) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("BinarySemaphore", semaphore, "LockedBy");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), lockedBy, quark);
	}
}
