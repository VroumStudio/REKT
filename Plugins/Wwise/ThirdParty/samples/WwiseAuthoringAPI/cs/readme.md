
# C# Samples
## Overview

The WaapiCS project is a Visual Studio 2017 project targeting .NET 4.5, which provides samples for the Wwise Authoring API using WampSharp.

## Requirements

You must first run Wwise and enable the Wwise Authoring API to be able to use it:

- In Wwise, select **Project > User Preferences**. (Default shortcut: Shift + U)
- In the Wwise Authoring API group box, select **Enable Wwise Authoring API**.
- Make sure you have `#unknown` in the allowed origins (there by default) or add `ws://127.0.0.1:8080` or any other address with the port.
- Click **OK**.

Visual Studio 2017 with C# support and the NuGet package manager extension is required.

## Setup
1. Go to the directory WaapiCS
2. Run `nuget restore` from a command line or open WaapiCS.sln, right-click on the solution and select "Restore NuGet Packages".
This will download the necessary dependencies such as WampSharp.
3. Build the project and observe that there are no errors in the main projects.

## Execution

Two samples can be run. To select the one to run, right-click on the WaapiCS project, select Properties, and in the Application tab select the appropriate sample under Startup object.

### WwiseCall

With Wwise running and a project opened, execute the solution and observe in the output:

	Calling 'ak.wwise.core.getInfo'
	Calling 'ak.wwise.ui.getSelectedObjects'
	ak.wwise.core.getInfo: Hello Wwise v2017.1.0
	ak.wwise.ui.getSelectedObjects: Got selected object data!
	ak.wwise.ui.getSelectedObjects: Got 1 object(s)!
	ak.wwise.ui.getSelectedObjects: The first selected object is 'New Sequence Container' ({03E8DC21-EB2F-4607-BFCC-25BCE69DFB27})
	ak.wwise.ui.getSelectedObjects: Its parent is 'ParentA' ({FBB64B3C-711C-46E6-9BBC-B49511F08244})

The received data from getSelectedObjects will vary based on your opened project. If no selection exists, the following message will be displayed:

	ak.wwise.ui.getSelectedObjects: Select something and try again!

### PubSubWwise

With Wwise running and a project opened, execute the solution and observe in the output:

	Add a child to an entity in the Wwise Authoring application.

Select the Default Work Unit under the Actor-Mixer Hierarchy in the Audio tab of the Project Explorer and add a container. You should see in the output:

	A child was added, ID={EAFD1799-41AA-4A98-B0CA-BDB8CDA0D878}
	Press any key to continue...
