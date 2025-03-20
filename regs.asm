.data

extern Global_OrigCreateFileReturnAddr:QWORD
extern Global_OrigReferenceAddrOne:QWORD
extern Global_OrigReferenceAddrTwo:QWORD
extern FinalStackVal:QWORD
extern index:DWORD
extern GetStackOffset:proc
extern FindStackPtr:proc
count DWORD 000h
countStub DWORD 000h

.code

    PUBLIC labelStubOne

    labelStubOne PROC 
        inc countStub         
        cmp countStub, 1 
        jne skipOne 
        mov index, 1h 
        jmp StackDeltaStub

    labelStubOne ENDP 

    PUBLIC labelStubTwo

    labelStubTwo PROC

        inc countStub      
        cmp countStub, 1
        jne skipTwo 
        mov index, 2h  
        jmp StackDeltaStub

    labelStubTwo ENDP 

    PUBLIC StackDeltaStub
    
    StackDeltaStub PROC
        
        push rcx 
        push rdx
        push r8
        push r9
        call FindStackPtr
        mov FinalStackVal, rsp
        call GetStackOffset
        pop r9
        pop r8
        pop rdx
        pop rcx
        cmp index, 1h
        je skipOne
        cmp index, 2h
        je skipTwo

    StackDeltaStub ENDP

    skipOne PROC

        mov rax, [Global_OrigReferenceAddrOne]
        jmp rax 

    skipOne ENDP

    skipTwo PROC

        mov rax, [Global_OrigReferenceAddrTwo]
        jmp rax

    skipTwo ENDP

end