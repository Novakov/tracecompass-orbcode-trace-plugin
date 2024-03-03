package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class MutexEvents {
	public static final int LOCK_STATE_CREATED = 0;
	public static final int LOCK_STATE_LOCKED = 10;
	public static final int LOCK_STATE_UNLOCKED = 11;

	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public MutexEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("MutexCreated")
	public void mutexCreated(@Field("mutex.name") String mutex) {
		setMutexState(fCurrentEvent, mutex, LOCK_STATE_CREATED);
		setMutexLockedBy(fCurrentEvent, mutex, null);
	}

	@EventHandler("MutexLocked")
	public void mutexLocked(@Field("mutex.name") String mutex, @Field("current_task.name") String lockedBy) {
		setMutexState(fCurrentEvent, mutex, LOCK_STATE_LOCKED);
		setMutexLockedBy(fCurrentEvent, mutex, lockedBy);
	}

	@EventHandler("MutexUnlocked")
	public void mutexUnlocked(@Field("mutex.name") String mutex) {
		setMutexState(fCurrentEvent, mutex, LOCK_STATE_UNLOCKED);
		setMutexLockedBy(fCurrentEvent, mutex, null);
	}

	public void setMutexState(ITmfEvent event, String mutex, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Mutex", mutex, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}

	public void setMutexLockedBy(ITmfEvent event, String mutex, String lockedBy) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Mutex", mutex, "LockedBy");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), lockedBy, quark);
	}
}
