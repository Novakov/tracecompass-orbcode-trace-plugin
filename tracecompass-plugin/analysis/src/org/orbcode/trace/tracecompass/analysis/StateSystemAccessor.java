package org.orbcode.trace.tracecompass.analysis;

import java.util.ArrayList;

import org.eclipse.jdt.annotation.NonNull;
import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.statesystem.core.statevalue.ITmfStateValue;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;

class StateSystemAccessor {
	private ITmfStateSystemBuilder fStateSystem;

	public StateSystemAccessor(ITmfStateSystemBuilder stateSystem) {
		this.fStateSystem = stateSystem;
	}
	
	public void setTaskState(ITmfEvent event, String taskName, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}
	
	public Integer getTaskState(String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "State");
		@NonNull
		ITmfStateValue ongoing = fStateSystem.queryOngoingState(quark);
		if(ongoing.isNull()) {
			return null;
		}
		
		return ongoing.unboxInt();
	}
	
	public void setTaskPreemptedCount(ITmfEvent event, String taskName, int newCount) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "PreemptedCount");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), newCount, quark);
	}
	
	public int getTaskPreemptedCount(String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "PreemptedCount");
		Object ongoing = fStateSystem.queryOngoing(quark);
		if(ongoing == null) {
			return 0;
		}
		
		return (int)ongoing;
	}
	
	public void setTaskBlockedOn(ITmfEvent event, String taskName, String objectType, String address, String operation) {
		int quarkBlockedOn = fStateSystem.getQuarkAbsoluteAndAdd("Task", taskName, "BlockedOn");
		int quarkObjectType = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Type");
		int quarkAddress = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Address");
		int quarkOperation = fStateSystem.getQuarkRelativeAndAdd(quarkBlockedOn, "Operation");
		
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), objectType, quarkObjectType);
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), address, quarkAddress);
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), operation, quarkOperation);
		
		ArrayList<String> parts = new ArrayList<String>();
		if(objectType != null) {
			parts.add(objectType);
		}
		if(address != null) {
			parts.add(address);
		}
		if(operation != null) {
			parts.add(operation);
		}
		
		if(parts.size() == 0) {
			fStateSystem.modifyAttribute(event.getTimestamp().getValue(), null, quarkBlockedOn);
		} else {
			String blockedOn = String.join("/", parts);
			fStateSystem.modifyAttribute(event.getTimestamp().getValue(), blockedOn, quarkBlockedOn);
		}
	}
	
	public void setReadyTaskCount(ITmfEvent event, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("ReadyTaskCount");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}
	
	public int getReadyTaskCount() {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("ReadyTaskCount");
		Object ongoing = fStateSystem.queryOngoing(quark);
		if(ongoing == null) {
			return 0;
		}
		
		return (int)ongoing;
	}
	
	public void setCurrentTask(ITmfEvent event, String taskName) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CurrentTask");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), taskName, quark);
	}

	public void setQueueCapacity(ITmfEvent event, String queue, int capacity) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Queue", queue, "Capacity");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), capacity, quark);
	}

	public void setQueueCount(ITmfEvent event, String queue, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Queue", queue, "Count");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}
	
	public void setBinarySemaphoreState(ITmfEvent event, String semaphore, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("BinarySemaphore", semaphore, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}
	
	public void setBinarySemaphoreLockedBy(ITmfEvent event, String semaphore, String lockedBy) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("BinarySemaphore", semaphore, "LockedBy");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), lockedBy, quark);
	}
	
	public void setMutexState(ITmfEvent event, String mutex, int state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Mutex", mutex, "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}
	
	public void setMutexLockedBy(ITmfEvent event, String mutex, String lockedBy) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Mutex", mutex, "LockedBy");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), lockedBy, quark);
	}

	public void setCountingSemaphoreCapacity(ITmfEvent event, String semaphore, int maxCount) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CountingSemaphore", semaphore, "Capacity");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), maxCount, quark);
	}

	public void setCountingSemaphoreCount(ITmfEvent event, String semaphore, int count) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CountingSemaphore", semaphore, "Count");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), count, quark);
	}
}
