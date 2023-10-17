package org.orbcode.trace.tracecompass.analysis;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;
import java.util.Objects;
import java.util.logging.Handler;

import org.eclipse.jdt.annotation.NonNull;
import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.statesystem.core.statevalue.ITmfStateValue;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.eclipse.tracecompass.tmf.core.statesystem.AbstractTmfStateProvider;
import org.eclipse.tracecompass.tmf.core.statesystem.ITmfStateProvider;
import org.eclipse.tracecompass.tmf.core.trace.ITmfTrace;

public class RTOSAnalysisStateProvider extends AbstractTmfStateProvider {
	@Target(ElementType.METHOD)
	@Retention(RetentionPolicy.RUNTIME)
	private @interface EventHandler {
		String value();
	}

	private static final @NonNull String PROVIDER_ID = "org.orbcode.trace.tracecompass.analysis.rtos.state.provider"; //$NON-NLS-1$
	private static final int VERSION = 1;

	private final HashMap<String, Method> fEventHandlers = new HashMap<>();

	private final int TASK_STATE_CREATED = 0;
	private final int TASK_STATE_RUNNING = 1;
	private final int TASK_STATE_READY = 4;

	public RTOSAnalysisStateProvider(ITmfTrace trace) {
		super(trace, PROVIDER_ID);

		for (Method method : getClass().getDeclaredMethods()) {
			EventHandler handledEvent = method.getAnnotation(EventHandler.class);

			if (handledEvent == null) {
				continue;
			}

			fEventHandlers.put(handledEvent.value(), method);
		}

	}

	@Override
	public ITmfStateProvider getNewInstance() {
		return new RTOSAnalysisStateProvider(getTrace());
	}

	@Override
	public int getVersion() {
		return VERSION;
	}

	@Override
	protected void eventHandle(ITmfEvent event) {
		ITmfStateSystemBuilder ss2 = Objects.requireNonNull(getStateSystemBuilder());

		StateSystemAccessor ss = new StateSystemAccessor(ss2);
		RTOSOperations rtos = new RTOSOperations(ss, event);

		Method handler = fEventHandlers.get(event.getName());
		if (handler == null) {
			int quark = ss2.getQuarkAbsoluteAndAdd("UnrecognizedEvents");
			@NonNull
			ITmfStateValue ongoing = ss2.queryOngoingState(quark);
			long unrecognized = 0;
			if (!ongoing.isNull()) {
				unrecognized = ongoing.unboxLong();
			}
			unrecognized++;
			ss2.modifyAttribute(event.getTimestamp().getValue(), unrecognized, quark);
			return;
		}

		try {
			handler.invoke(this, event, rtos);
		} catch (IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
			throw new RuntimeException(e);
		}
	}

	@EventHandler("task_created")
	private void onTaskCreated(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		rtos.taskCreated(taskName);
	}

	@EventHandler("task_switched_in")
	private void onTaskSwitchedIn(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		rtos.taskSwitchedIn(taskName);
	}

	@EventHandler("task_switched_out")
	private void onTaskSwitchedOut(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		String outStateName = event.getContent().getFieldValue(String.class, "OutState");

		if (outStateName.equals("Ready")) {
			rtos.taskSwitchedOutReady(taskName);
		} else if (outStateName.equals("Blocked")) {
			String objectType = event.getContent().getFieldValue(String.class, "BlockedOn");
			String address = event.getContent().getFieldValue(String.class, "BlockedOnObject");
			String operation = event.getContent().getFieldValue(String.class, "BlockingOperation");

			rtos.taskSwitchedOutBlocked(taskName, objectType, address, operation);
		} else if (outStateName.equals("Delayed")) {
			rtos.taskSwitchedOutDelayed(taskName);
		}
	}

	@EventHandler("task_readied")
	private void onTaskReadied(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		rtos.taskReadied(taskName);
	}

	@EventHandler("task_notified")
	private void onTaskNotified(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		int index = Integer.parseInt(event.getContent().getFieldValue(String.class, "NotifyIndex"));
		int updatedValue = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedValue"));
		
		rtos.taskNotified(taskName, index, updatedValue);
	}
	
	@EventHandler("task_notify_received")
	private void onTaskNotifyReceied(ITmfEvent event, RTOSOperations rtos) {
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");
		int index = Integer.parseInt(event.getContent().getFieldValue(String.class, "NotifyIndex"));
		int updatedValue = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedValue"));
		
		rtos.taskNotifyReceived(taskName, index, updatedValue);
	}

	@EventHandler("queue_created")
	private void onQueueCreated(ITmfEvent event, RTOSOperations rtos) {
		String queue = event.getContent().getFieldValue(String.class, "Queue");
		int capacity = Integer.parseInt(event.getContent().getFieldValue(String.class, "Capacity"));

		rtos.queueCreated(queue, capacity);
	}

	@EventHandler("queue_pushed")
	private void onQueuePushed(ITmfEvent event, RTOSOperations rtos) {
		String queue = event.getContent().getFieldValue(String.class, "Queue");
		int newCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedItemsCount"));

		rtos.queuePushed(queue, newCount);
	}

	@EventHandler("queue_popped")
	private void onQueuePopped(ITmfEvent event, RTOSOperations rtos) {
		String queue = event.getContent().getFieldValue(String.class, "Queue");
		int newCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedItemsCount"));

		rtos.queuePopped(queue, newCount);
	}

	@EventHandler("binary_semaphore_created")
	private void onBinarySemaphoreCreated(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");

		rtos.binarySemaphoreCreated(semaphore);
	}

	@EventHandler("binary_semaphore_locked")
	private void onBinarySemaphoreLocked(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");

		rtos.binarySemaphoreLocked(semaphore, taskName);
	}

	@EventHandler("binary_semaphore_unlocked")
	private void onBinarySemaphoreUnlocked(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");

		rtos.binarySemaphoreUnlocked(semaphore);
	}

	@EventHandler("counting_semaphore_created")
	private void onCountingSemaphoreCreated(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");
		int maxCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "MaxCount"));
		int initialCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "InitialCount"));

		rtos.countingSemaphoreCreated(semaphore, maxCount, initialCount);
	}

	@EventHandler("counting_semaphore_given")
	private void onCountingSemaphoreGiven(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");
		int updatedCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedCount"));

		rtos.countingSemaphoreGiven(semaphore, updatedCount);
	}

	@EventHandler("counting_semaphore_taken")
	private void onCountingSemaphoreTaken(ITmfEvent event, RTOSOperations rtos) {
		String semaphore = event.getContent().getFieldValue(String.class, "Semaphore");
		int updatedCount = Integer.parseInt(event.getContent().getFieldValue(String.class, "UpdatedCount"));

		rtos.countingSemaphoreTaken(semaphore, updatedCount);
	}

	@EventHandler("mutex_created")
	private void onMutexCreated(ITmfEvent event, RTOSOperations rtos) {
		String mutex = event.getContent().getFieldValue(String.class, "Mutex");

		rtos.mutexCreated(mutex);
	}

	@EventHandler("mutex_locked")
	private void onMutexLocked(ITmfEvent event, RTOSOperations rtos) {
		String mutex = event.getContent().getFieldValue(String.class, "Mutex");
		String taskName = event.getContent().getFieldValue(String.class, "TaskName");

		rtos.mutexLocked(mutex, taskName);
	}

	@EventHandler("mutex_unlocked")
	private void onMutexUnlocked(ITmfEvent event, RTOSOperations rtos) {
		String mutex = event.getContent().getFieldValue(String.class, "Mutex");

		rtos.mutexUnlocked(mutex);
	}
	
	@EventHandler("exception_entered") 
	private void onExceptionEntered(ITmfEvent event, RTOSOperations rtos) {
		int exceptionNumber = Integer.parseInt(event.getContent().getFieldValue(String.class, "ExceptionNumber"));
		
		rtos.exceptionEntered(exceptionNumber);
	}
	
	@EventHandler("exception_exited") 
	private void onExceptionExited(ITmfEvent event, RTOSOperations rtos) {
		int exceptionNumber = Integer.parseInt(event.getContent().getFieldValue(String.class, "ExceptionNumber"));
		
		rtos.exceptionExited(exceptionNumber);
	}
	
	@EventHandler("exception_returned") 
	private void onExceptionReturned(ITmfEvent event, RTOSOperations rtos) {
		int exceptionNumber = Integer.parseInt(event.getContent().getFieldValue(String.class, "ExceptionNumber"));
		
		rtos.exceptionReturned(exceptionNumber);
	}
}
