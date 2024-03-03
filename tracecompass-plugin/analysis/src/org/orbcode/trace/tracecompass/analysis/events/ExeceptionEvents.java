package org.orbcode.trace.tracecompass.analysis.events;

import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.orbcode.trace.tracecompass.analysis.EventHandler;
import org.orbcode.trace.tracecompass.analysis.Field;

public class ExeceptionEvents {
	public static final int EXCEPTION_STATE_ACTIVE = 100;

	private ITmfStateSystemBuilder fStateSystem;
	private ITmfEvent fCurrentEvent;

	public ExeceptionEvents(ITmfStateSystemBuilder stateSystem, ITmfEvent currentEvent) {
		fStateSystem = stateSystem;
		fCurrentEvent = currentEvent;
	}

	@EventHandler("ExceptionEntered")
	public void exceptionEntered(@Field("exception_number") int exceptionNumber) {
		setExceptionState(fCurrentEvent, exceptionNumber, EXCEPTION_STATE_ACTIVE);
		setCurrentExceptionNumber(fCurrentEvent, exceptionNumber);
	}

	@EventHandler("ExceptionExited")
	public void exceptionExited(@Field("exception_number") int exceptionNumber) {
		setExceptionState(fCurrentEvent, exceptionNumber, null);
	}

	@EventHandler("ExceptionReturned")
	public void exceptionReturned(@Field("exception_number") int exceptionNumber) {
		setCurrentExceptionNumber(fCurrentEvent, exceptionNumber);
	}
	
	private void setExceptionState(ITmfEvent event, int exceptionNumber, Integer state) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("Exception", Integer.toString(exceptionNumber), "State");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), state, quark);
	}
	
	private void setCurrentExceptionNumber(ITmfEvent event, int exceptionNumber) {
		int quark = fStateSystem.getQuarkAbsoluteAndAdd("CurrentExceptionNumber");
		fStateSystem.modifyAttribute(event.getTimestamp().getValue(), exceptionNumber, quark);
	}
}
