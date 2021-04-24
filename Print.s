; Print.s
; Student names: James Mahon and Ngoc Dao
; Last modification date: change this to the last modification date or look very silly
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

cnt		EQU		0
n		EQU		4
FP		RN		11

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
		PUSH 	{FP, LR}
		SUB		SP, #8					; allocate for n, cnt
		MOV		FP, SP					; Frame Pointer = Stack Pointer
		MOV		R2, #10					; R2 = 10 = divisor
		STR		R0, [FP, #n]
		MOV		R0, #0					
		STR		R0, [FP, #cnt]			; cnt = 0
ODloop	
		LDR		R0, [FP, #n]			; R4 = n
		UDIV	R3, R0, R2				; R3 = n/10, were n is 32-bit unsigned input
		MUL		R1, R3, R2				; R1 = n/10*10
		SUB		R1, R0, R1				; R1 = n%10
		STR		R0, [FP, #n]			
		PUSH	{R1}					; save value
		LDR		R0, [FP, #cnt]			; R4 = cnt
		ADD		R0, #1					; cnt++
		STR		R0, [FP, #cnt]		
		LDR		R0, [FP, #n]			
		MOVS	R0, R3					; R0 n = n/10
		STR		R0, [FP, #n]
		CMP		R0, #0					; is n == 0?
		BNE		ODloop					; if not continue
		
ODout	POP		{R0}					; restore into R0
		ADD		R0, R0, #'0'			; convert ASCII
		BL		SSD1306_OutChar			; print character
		LDR		R0, [FP, #cnt]			; R4 = cnt
		SUBS	R0, #1					; cnt--
		STR		R0, [FP, #cnt]
		CMP		R0, #0					; is cnt == 0?
		BNE		ODout					; if not, loop
		ADD		SP, #8					; deallocate
		POP 	{FP, LR}

		BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
		PUSH 	{FP, LR}
		CMP		R0, #1000 			; is N >= 1000?;		
		BHS		OutRange			; if so, input is out of range
		
*****************************************************************************
; Copy paste from OutDec, changed a few lines (check comments)
		SUB		SP, #8					; allocate for n, cnt
		MOV		FP, SP					; Frame Pointer = Stack Pointer
		MOV		R2, #10					; R2 = 10 = divisor
		STR		R0, [FP, #n]
		MOV		R0, #0					
		STR		R0, [FP, #cnt]			; cnt = 0
OFloop	
		LDR		R0, [FP, #n]			; R4 = n
		UDIV	R3, R0, R2				; R3 = n/10, were n is 32-bit unsigned input
		MUL		R1, R3, R2				; R1 = n/10*10
		SUB		R1, R0, R1				; R1 = n%10
		STR		R0, [FP, #n]			
		PUSH	{R1, R4}				; save value
		LDR		R0, [FP, #cnt]			; R4 = cnt
		ADD		R0, #1					; cnt++
		STR		R0, [FP, #cnt]		
		LDR		R0, [FP, #n]			
		MOVS	R0, R3					; R0 n = n/10
		STR		R0, [FP, #n]
		LDR		R0, [FP, #cnt]			
		CMP		R0, #3					; is cnt == 3?								--changed--
		BNE		OFloop					; if not, loop								
		
OFout	POP		{R0, R4}				; restore into R0
		ADD		R0, R0, #'0'			; convert ASCII
		BL		SSD1306_OutChar			; print character
		LDR		R0, [FP, #cnt]			; R4 = cnt
		SUBS	R0, #1					; cnt--
		STR		R0, [FP, #cnt]
		CMP		R0, #2					; is cnt == 2?								--added--
		BNE		skip1					; if not, skip								--added--
		MOV		R0, #0x2E				; Output '.' character						--added--
		BL		SSD1306_OutChar														
		LDR		R0, [FP, #cnt]
skip1	CMP		R0, #0					; is cnt == 0?
		BNE		OFout					; if not continue
		ADD		SP, #8					; deallocate
******************************************************************************
		B		OFexit
		
OutRange
		MOV		R0, #0x2A			; * in ASCII
		BL		SSD1306_OutChar		; print character
		MOV		R0, #0x2E			; . in ASCII
		BL		SSD1306_OutChar		; print character
		MOV		R0, #0x2A			; * in ASCII
		BL		SSD1306_OutChar		; print character
		MOV		R0, #0x2A			; * in ASCII
		BL		SSD1306_OutChar		; print character
		
OFexit	POP 	{FP, LR}
		BX   	LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file
