TraceCompass plugin
===

# Building from command line
## Prerequisites
* Java 17 (https://adoptium.net/)
* Maven 3.9.3

# Build steps
1. Clone the repository
2. Go to `tracecompass-plugin` folder
3. Run `mvn site`

Eclipse update site will be located in `tracecompass-plugin/target/repository`.

# Installing plugin in TraceCompass
1. Open TraceCompass
2. Go to Help -> Install New Software...
3. Click Add...
4. Click Local...
5. Select `tracecompass-plugin/target/repository` folder
6. Select `Trace Analysis plugin` from the list
7. Click Next and follow TraceCompass prompts (including trusing plugin source)


# Building in Eclipse
## Prerequisites
* Eclipse IDE for Eclipse Committers  (https://www.eclipse.org/downloads/packages/)

## Build steps
1. Open Eclipse and create new workspace
2. Click File -> Import...
3. Select Maven -> Existing Maven Projects
4. Click Next
5. Select `tracecompass-plugin` folder
6. Click Finish
8. Open file `target-platform` in `target-platform` project
9. Click `Set as active platfrom target`

In order to run TraceCompass with plugin directly from Eclipse:
1. Create new Eclipse Application run configuration
2. In `Main` tab select `org.eclipse.tracecompass.rcp.branding.product` as `Run a product`
3. Click Run