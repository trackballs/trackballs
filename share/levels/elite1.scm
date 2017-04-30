; This classic will go down in history like  "War and Peace". ;-)

(set-track-name "Castle of Darius the Mede")
; A historical figure from The Book of Daniel - chapter 6.

(set-author "Dietrich Radel")

; Nice atmospheric fog. Sets the scene. Insert theatrical mood music here.
(fog)
(fog-color 0.3 0.2 0.3)

; Absurd speed for moving spikes, but I never intended anyone (except The Flash
; to be able to pass through these. Most superpower-impaired folk will opt to
; slow these down by flicking a switch, or by pouring water into their ATX case.
(define speed 3.0)


(set-start-position 189.5 155.5)

; Give 'em a generous helping of time.
(cond
 ((= (difficulty) *easy*)
         (start-time 400)
 )
 ((= (difficulty) *normal*)
         (start-time 300)
 )
 ((= (difficulty) *hard*)
         (start-time 250)
 )
)


; Add the goal and a savepoint, before I forget.
(diamond 236.5 230.5)
(add-goal 226 231 #f "")


; Just after the battle, the fog gets thicker. Insert scary dischordant violin music here.
(trigger 205 254 0.5 (lambda () (fog 1.5)))


; If player tries to pull a swifty by taking the shortcut, whoops - out go the lights.
(trigger 234 244 1.0 (lambda () (night)))
(trigger 235 244 1.0 (lambda () (night)))
(trigger 236 244 1.0 (lambda () (night)))
(trigger 237 244 1.0 (lambda () (night)))


; Ok, I'll turn the lights back on at top of moving platforms. I'll call it 'grace'.
(trigger 250 251 1.0 (lambda () (fog 1.5)))



(define forestff (forcefield 207.0 253.0 0.0 0.0 1.0 0.0 1.5 *ff-bounce*))

; Switch on island that makes pipe appear and also turns off the forcefield.
; Many thanks to Mathias Broxvall for helping me with this function. =)
(define pipe0 #f)
(switch 244.5 180.5
    (lambda ()
       (set-onoff forestff #t)
    )
    (lambda ()
       (if (not pipe0)
         (begin
           (set! pipe0 #t)
           (let ((pipe-island (pipe 244 183.5 -7.5 244 188.2 -7.5 0.5)))
			 (set-wind pipe-island 2.0 2.0)
			 (set-primary-color pipe-island 0.5 1.0 0.5 0.5)
			 )
           (set-onoff forestff #f)
		   )
		 )
	   )
	)





; Some forcefields and their overly distant switches.
(define doorff (forcefield 225.0 233.0 0.0 0.0 2.0 0.0 3.5 *ff-bounce*))
(switch 208.5 254.5 (lambda () (set-onoff doorff #t)) (lambda () (set-onoff doorff #f)))
(define doorff2 (forcefield 226.0 233.0 0.0 0.0 2.0 0.0 3.5 *ff-bounce*))
(switch 254.5 203.5 (lambda () (set-onoff doorff2 #t)) (lambda () (set-onoff doorff2 #f)))


; I guess I'd better be nice and give 'em a save point here. Aren't I nice? Well they earned it.
(diamond 244.5 191.5)

; Groovy moving platforms. Sheesh, I'd hate to do this in the dark.
(add-cyclic-platform 250 250 250 250 -3. 3. 6. .5)
(add-cyclic-platform 250 249 250 249 -4. 1. 4. .5)
(add-cyclic-platform 250 248 250 248 -5. 0. 6. .5)
(add-cyclic-platform 250 247 250 247 -5. -2. 4. .5)
(add-cyclic-platform 250 246 250 246 -8. -4. 6. .5)

; For the Homer Simpsons out ther, a sign to warn that icy pond will kill the marble.
(set-primary-color (sign "Danger!" 1.0 15.0 0.0 232.5 181.5) 0.8 0.8 0.4)

; Not just one, but a whole bunch of Mr Black's! Muhahahahahah!!!!!
(define mr-black1 (new-mr-black 197.5 237.5))
(define mr-black2 (new-mr-black 200.5 253.5))
(define mr-black3 (new-mr-black 197.5 253.5))
(define mr-black4 (new-mr-black 200.5 237.5))
(define mr-black5 (new-mr-black 197.5 244.5))
(define mr-black6 (new-mr-black 197.5 227.5))
(define mr-black7 (new-mr-black 200.5 227.5))
(define mr-black8 (new-mr-black 197.5 220.5))
(define mr-black9 (new-mr-black 200.5 220.5))

; Give them all stupidly fast acceleration - well, not too fast I guess.
(set-acceleration mr-black1 3.0)
(set-acceleration mr-black2 5.0)
(set-acceleration mr-black4 3.0)
(set-acceleration mr-black5 3.0)
(set-acceleration mr-black6 5.0)
(set-acceleration mr-black8 3.0)


(cond
 ((= (difficulty) *easy*)
         (set-acceleration mr-black7 4.0)
         (set-acceleration mr-black3 6.0)
         (set-acceleration mr-black9 8.0)
         (add-modpill 191.5 253.5 *mod-spike* 40 40)
 )
 ((= (difficulty) *normal*)
         (set-acceleration mr-black7 6.0)
         (set-acceleration mr-black3 8.0)
         (set-acceleration mr-black9 10.0)
 )
 ((= (difficulty) *hard*)
         (set-acceleration mr-black7 8.0)
         (set-acceleration mr-black3 10.0)
         (set-acceleration mr-black9 12.0)
 )
)


; Invisble Mr Black on top of the castle - heheheh
(define invisible 'true)
(switch 226.5 236.5 (lambda ()() )  (lambda () (if invisible (begin (set! invisible #f) (set! mr-black1 (new-mr-black 225.5 250.5))(set-speed spike 0.2)))) )
(define spike (add-spike 251.5 219.5 speed -0.20))

; Shute leading you outside
(define pipe-shute (pipe 251.5 218.5 2.4 251.5 207.0 -6.0 0.7) )
(set-wind pipe-shute 2.0 2.0)



; Spikes to help avoid any Mr Blacks that just won't go away.
(add-modpill 254.5 204.5 *mod-spike* 25 40)

; A bunch of pretty waving flags to distract the player and eat up time.
(add-flag 241 246 200 1 0.1)
(add-flag 249 230 250 1 0.1)
(add-flag 215 205 150 1 0.1)
(add-flag 247 196 750 1 0.1) ; Ok, I guess this one is worth getting though.
(set-primary-color (sign "Eternity" 1.0 15.0 0.0 249.5 196.5) 0.8 0.4 0.8)
(add-flag 239 181 150 1 0.1)
(add-flag 223 181 250 1 0.1)
(add-flag 190 192 150 1 0.1)
(add-flag 202 217 150 1 0.1)
(add-flag 190 243 250 1 0.1)
(add-flag 197 253  25 1 0.1) ; Hmmmm, why did I bother? Sucker! :p
(add-flag 229 252 150 1 0.1)
(add-flag 229 252 150 1 0.1)
(add-flag 248 215 500 1 0.1) ; Oh ok, grab this one also - if you don't burn to many calories to get it.

; Secret level....ssshhhh!!!!!!!
(define pipe-secret(pipe 254.5 171.3 -9.2 254.3 157.0 -8.9 0.7))
(set-wind pipe-secret 5.0 5.0)
(add-flag 254 150 250 1 0.1)
(add-flag 254 148 250 1 0.1)
(add-flag 254 146 250 1 0.1)
(add-flag 250 145 250 1 0.1)
(add-flag 250 150 250 1 0.1)
(add-flag 250 155 250 1 0.1)
(add-flag 248 150 250 1 0.1)
(add-flag 245 150 250 1 0.1)
(add-flag 245 155 250 1 0.1)



; "For the wages of sin is death, but the gift of God is eternal
;  life in Jesus Christ our Lord" - Romans 6:23
