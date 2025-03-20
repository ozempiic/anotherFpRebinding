
### StealthHook Based PoC
Original PoC by x86matthew's [StealthHook](https://www.x86matthew.com/view_post?id=stealth_hook)

Initially i didnt intend to post this so there will be some redundancy, like the duplication of similar functions and perchance some errors but i'll fix that soon™

-------

###### Tested the following on windows 7, windows 10 and windows 11. All 64 bit.

x86matthew's stealthHook appeared really cool to me but it used single step exception on every instruction which seemed a bit slow and i presumed that excessive calls to VEH would essentially be flagged so i tried to circumvent the use of hardware breakpoints and VEH in my implementation

- In no way has my implementation been tailored to bypass EDRs in real world cases, if it does yippie, if not then oh well.

---------

### How it works?
My implementation works by taking a pre-exsting dll (kernelbase in this case) and searching its .data section which is usually RW marked —crucial to note that this technique wont work if the .data section is not RW marked— and deferences each set of instructions (most of these will get dereferenced to invalid addresses) and then takes the valid memory addresses which fall under the memory range of the dll, to validate the deferenced pointer we again dereference it and check for a near absolute jmp

          +-----------------------------------------------------------------------------+
          | kernelbase.dll (Base Address: 0x10000000)                                   |
          | DllSize: 0x0001000                                                          |
          |-----------------------------------------------------------------------------|
          | .data Section (Read/Write)                                                  |
          |                                                                             |
          |                                                                             |
          |   [0xff00aabb]  -->  Invalid                                                |
          |                                                                             |
          |   [0xffb9240b]  -->  [Valid]               --> check for FF 25 XX XX XX     |
          |                      ( valid because between BA and BA + DllSize)           |
          |                                                                             |
          |   [0xffe900c8]  -->  Invalid                                                |
          |                                                                             |
          +-----------------------------------------------------------------------------+

I hardcoded two label stubs and correponding functions in resonance with them because on win7, win10 and win11 i found only two .data pointers RtlAnsiStringToUnicodeString and RtlUnicodeStringToAnsiString that were validated. 

#### For calculating the stack delta 

 - I calculated the address on sreach on stack with _ReturnAddress intrinsic right before calling the targetwinapi and searched for near absolute call opcode from that address onwards.
 - The GetStackOffset function then loops through the stack pointer incrementally, derefencing the stack pointer to search for the return address calculated in the above line.

------------
### Execution Flow
-  The first WinAPI call sets up the prerequisites.
-  The second WinAPI call executes the implant.

------------
### improve this?
- this can be improved by first and foremost removing the hardcoded functions and using an array of structs to handle the hooking logic 
- call stacking spoofing because the implant will be executed from an incomplete winapi call
- ngl i cant think of anything else 

------------
### References
I had used stardust as a testing binary and also the LoadFileIntoMemory func is from stardust 
 - [Stardust](https://github.com/Cracked5pider/Stardust)
 -----------
