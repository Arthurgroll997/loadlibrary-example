# loadlibrary-example
This example shows how to load a dll calling LoadLibrary to load the target DLL

# Tested platforms
This method was tested in a x86 process with a x86 dll (obviously)

# The process and theory
The process behind this DLL injection is quite simple:
We will use the LoadLibrary function to load a dll, to do so we will allocate memory in the target process (we will use that as a parameter to the LoadLibrary function), write the location of the dll and create a new thread calling the function and passing the memory location that we allocated containing the location of our dll. If these steps work, DllMain will be called and everything should work fine.

# Setting up
You can choose between two methods to get the target processes name, you can either set the process name in the code (setting the pre-processor RAW_INPUT to 1) or create a input.txt file containing the target process name in the injector directory seting the pre-processor RAW_INPUT to 0.

# How to use
Open the process you want to inject the DLL, and then just move the DLL to the injector.

# Important notes

If you're using this injector to inject a x86 dll in a x86 process, compile with the x86 platform too.
