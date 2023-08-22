package org.orbcode.trace.tracecompass.analysis;
import org.eclipse.tracecompass.tmf.core.statesystem.ITmfStateProvider;
import org.eclipse.tracecompass.tmf.core.statesystem.TmfStateSystemAnalysisModule;

public class RTOSAnalysisModule extends TmfStateSystemAnalysisModule {

	@Override
	protected ITmfStateProvider createStateProvider() {
		return new RTOSAnalysisStateProvider(getTrace());
	}

}
