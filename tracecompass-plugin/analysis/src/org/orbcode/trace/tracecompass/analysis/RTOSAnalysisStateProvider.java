package org.orbcode.trace.tracecompass.analysis;

import java.lang.annotation.Annotation;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Objects;

import org.eclipse.jdt.annotation.NonNull;
import org.eclipse.tracecompass.statesystem.core.ITmfStateSystemBuilder;
import org.eclipse.tracecompass.statesystem.core.statevalue.ITmfStateValue;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.eclipse.tracecompass.tmf.core.statesystem.AbstractTmfStateProvider;
import org.eclipse.tracecompass.tmf.core.statesystem.ITmfStateProvider;
import org.eclipse.tracecompass.tmf.core.trace.ITmfTrace;
import org.orbcode.trace.tracecompass.analysis.events.BinarySemaphoreEvents;
import org.orbcode.trace.tracecompass.analysis.events.CountingSemaphoreEvents;
import org.orbcode.trace.tracecompass.analysis.events.ExeceptionEvents;
import org.orbcode.trace.tracecompass.analysis.events.MutexEvents;
import org.orbcode.trace.tracecompass.analysis.events.QueueEvents;
import org.orbcode.trace.tracecompass.analysis.events.TaskEvents;
import org.orbcode.trace.tracecompass.analysis.events.TaskNotifyEvents;

public class RTOSAnalysisStateProvider extends AbstractTmfStateProvider {
	private static final @NonNull String PROVIDER_ID = "org.orbcode.trace.tracecompass.analysis.rtos.state.provider"; //$NON-NLS-1$
	private static final int VERSION = 1;

	private final HashMap<String, Method> fEventHandlers = new HashMap<>();

	public RTOSAnalysisStateProvider(ITmfTrace trace) {
		super(trace, PROVIDER_ID);

		ArrayList<Class<?>> handlerClasses = new ArrayList<>();

		handlerClasses.add(TaskEvents.class);
		handlerClasses.add(ExeceptionEvents.class);
		handlerClasses.add(QueueEvents.class);
		handlerClasses.add(BinarySemaphoreEvents.class);
		handlerClasses.add(CountingSemaphoreEvents.class);
		handlerClasses.add(TaskNotifyEvents.class);
		handlerClasses.add(MutexEvents.class);

		for (Class<?> handlerClass : handlerClasses) {
			for (Method method : handlerClass.getDeclaredMethods()) {
				EventHandler handledEvent = method.getAnnotation(EventHandler.class);

				if (handledEvent == null) {
					continue;
				}

				fEventHandlers.put(handledEvent.value(), method);
			}
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
		ITmfStateSystemBuilder stateSystem = Objects.requireNonNull(getStateSystemBuilder());

		Method handler = fEventHandlers.get(event.getName());
		if (handler == null) {
			onUnrecognizedEvent(event, stateSystem);

			return;
		}

		Object[] params = resolveHandlerParams(handler, event);

		Class<?> handlerClass = handler.getDeclaringClass();
		try {
			Constructor<?> ctorDirect = handlerClass.getDeclaredConstructor(ITmfStateSystemBuilder.class,
					ITmfEvent.class);
			Object handlerObject = ctorDirect.newInstance(stateSystem, event);

			handler.invoke(handlerObject, params);

		} catch (NoSuchMethodException | SecurityException | InstantiationException | IllegalAccessException
				| IllegalArgumentException | InvocationTargetException e) {
			throw new RuntimeException(e);
		}
	}

	private void onUnrecognizedEvent(ITmfEvent event, ITmfStateSystemBuilder stateSystem) {
		int quark = stateSystem.getQuarkAbsoluteAndAdd("UnrecognizedEvents");
		@NonNull
		ITmfStateValue ongoing = stateSystem.queryOngoingState(quark);
		long unrecognized = 0;
		if (!ongoing.isNull()) {
			unrecognized = ongoing.unboxLong();
		}
		unrecognized++;
		stateSystem.modifyAttribute(event.getTimestamp().getValue(), unrecognized, quark);
	}

	private Object convertFieldValue(Class<?> expectedType, String fieldValue) {
		if (expectedType == String.class) {
			return fieldValue;
		}
		if (expectedType == int.class) {
			return (int) Integer.parseInt(fieldValue);
		}
		if (expectedType == boolean.class) {
			return fieldValue.equals("True");
		}

		return null;
	}

	private Object[] resolveHandlerParams(Method handler, ITmfEvent event) {
		Object[] params = new Object[handler.getParameterCount()];
		Annotation[][] paramAnnotations = handler.getParameterAnnotations();
		Class<?>[] paramTypes = handler.getParameterTypes();

		for (int i = 0; i < params.length; i++) {
			Field fieldAnnotation = null;
			for (Annotation item : paramAnnotations[i]) {
				if (item.annotationType() == Field.class) {
					fieldAnnotation = (Field) item;
					break;
				}
			}
			if (fieldAnnotation == null) {
				params[i] = null;
			}

			String fieldValue = event.getContent().getFieldValue(String.class, fieldAnnotation.value());
			params[i] = convertFieldValue(paramTypes[i], fieldValue);
		}

		return params;
	}

}
