                .486
                .MODEL  flat
                LOCALS

                .DATA
                EXTRN _PITCH:DWORD,_SCREENX:DWORD,_SCREENY:DWORD

                .CODE

					 PUBLIC _draw_tile_asm


_draw_tile_asm  PROC  NEAR
                ARG @@B,@@DEST,@@X,@@Y

               push 	ebp
               mov	ebp,esp
               push  ebx

         ;;clipping checks
        xor     eax,eax
        mov     ecx,@@X
        mov     edx,@@Y

        add     ecx,8 ;right extent of image
        add     edx,8 ;bottom extent of image

        cmp     [@@X],eax    ;left clip
        jl      draw_tile_asm_clip
        cmp     [_SCREENX],ecx ;right clip
        jl      draw_tile_asm_clip
        cmp     [@@Y],eax      ;top clip
        jl      draw_tile_asm_clip
        cmp     [_SCREENY],edx ;bottom clip
        jl      draw_tile_asm_clip


;;--------------------------
;;normal drawing
					push	esi
               push	edi

               mov	eax,[@@Y]
					mul	[_PITCH]
               add	eax,[@@X]
               mov	edi,[@@DEST]
					mov	esi,[@@B] ;get pointer to tile
               add	edi,eax ;di->dest si->tile
               mov	ecx,8

@@LP:          mov	eax,[esi]
               mov	ebx,[esi+4]
               mov	[edi],eax
               mov	[edi+4],ebx
               add	edi,_PITCH
               add	esi,8
               dec	ecx
               jnz	@@LP
               pop	edi
               pop 	esi

@@DONE:        pop   ebx
               pop	ebp
               ret
_draw_tile_asm  ENDP




draw_tile_asm_clip  PROC
        ARG @@B,@@DEST,@@X,@@Y

        push	esi
        push	edi
        mov	   esi,@@B
        mov	   edi,@@DEST
;        jmp  @@DONE

;top clip
        xor     eax,eax
        xor     ebx,ebx
        cmp     [@@Y],eax
        jg      @@NOTOP
        cmp     edx,eax ;top edge is <0
        jle     @@DONE
        sub     eax,@@Y ;ystart=0-Y
        add     @@Y,eax ;Y=0
        sub		 ebx,eax     ;;save ystart
        shl	  	 eax,3      ;;move source pointer down
        add		 esi,eax
@@NOTOP:

;bottom clip
        mov     eax,8 ;YTOTAL
        sub     edx,[_SCREENY]
        jle     @@NOBOTTOM
        sub     eax,edx ;ylen=SCREENY-Y
        jle     @@DONE
@@NOBOTTOM:
        lea		edx,[eax+ebx] ;YLEN=YTOTAL-YSTART

;left clip
        xor     eax,eax
        xor 	 ebx,ebx
        cmp     [@@X],eax
        jg      @@NOLEFT
        cmp     ecx,eax ;right edge is <0
        jle     @@DONE
        sub     eax,@@X   ;xstart=0-X
        add     [@@X],eax ;X=0
        add		 esi,eax    ;;move source pointer right
        sub 	 ebx,eax  ;save -xstart
@@NOLEFT:

;right clip
        mov     eax,8
        sub     ecx,[_SCREENX]
        jle     @@NORIGHT
        sub     eax,ecx
        jle     @@DONE
@@NORIGHT:
		  add     ebx,eax ;store XLEN=XTOTAL + (-XSTART)


        ;point to memory for dest
        push   edx
        mov     eax,_PITCH
        mul     @@Y
        add     eax,@@X
        add     edi,eax
        pop		 edx

     ;;draw it!
@@LP:   mov		ecx,ebx ;xlen
        push   esi

        rep		movsb ;copy scanline

        pop    esi   ;advance esi
        add		edi,_PITCH ;advance edi

        add		esi,8
        sub		edi,ebx

        dec		edx
        jg   	@@LP


@@DONE: pop		edi
        pop		esi
        pop    ebx
  		  pop    ebp
		  ret
draw_tile_asm_clip  ENDP




					END












