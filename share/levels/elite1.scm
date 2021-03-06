; This classic will go down in history like  "War and Peace". ;-)

(set-track-name (_ "Castle of Darius the Mede"))
; A historical figure from The Book of Daniel - chapter 6.

(set-author "Dietrich Radel")

; Nice atmospheric fog. Sets the scene. Insert theatrical mood music here.
(fog)
(fog-color 0.3 0.2 0.3)

; Absurd speed for moving spikes, but I never intended anyone (except The Flash
; to be able to pass through these. Most superpower-impaired folk will opt to
; slow these down by flicking a switch, or by pouring water into their ATX case.
(define speed 3.0)


(set-start-position 188 154)

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
(diamond 235 229)
(add-goal 225 230 #f "")


; Just after the battle, the fog gets thicker. Insert scary dischordant violin music here.
(trigger 203.5 252.5 0.5 (lambda () (fog 1.5)))


; If player tries to pull a swifty by taking the shortcut, whoops - out go the lights.
(trigger 232.5 242.5 1.0 (lambda () (night)))
(trigger 233.5 242.5 1.0 (lambda () (night)))
(trigger 234.5 242.5 1.0 (lambda () (night)))
(trigger 235.5 242.5 1.0 (lambda () (night)))


; Ok, I'll turn the lights back on at top of moving platforms. I'll call it 'grace'.
(trigger 248.5 249.5 1.0 (lambda () (fog 1.5)))



(define forestff (forcefield 205.5 251.5 0.0 0.0 1.0 0.0 1.5 *ff-bounce*))

; Switch on island that makes pipe appear and also turns off the forcefield.
; Many thanks to Mathias Broxvall for helping me with this function. =)
(define pipe0 #f)
(switch 243 179
    (lambda ()
       (set-onoff forestff #t)
    )
    (lambda ()
       (if (not pipe0)
         (begin
           (set! pipe0 #t)
           (let ((pipe-island (pipe 242.5 182 -7.5 242.5 186.7 -7.5 0.5)))
       (set-wind pipe-island 2.0 2.0)
       (set-primary-color pipe-island 0.5 1.0 0.5 0.5)
       )
           (set-onoff forestff #f)
       )
     )
     )
  )





; Some forcefields and their overly distant switches.
(define doorff (forcefield 223.5 231.5 0.0 0.0 2.0 0.0 3.5 *ff-bounce*))
(switch 207 253 (lambda () (set-onoff doorff #t)) (lambda () (set-onoff doorff #f)))
(define doorff2 (forcefield 224.5 231.5 0.0 0.0 2.0 0.0 3.5 *ff-bounce*))
(switch 253 202 (lambda () (set-onoff doorff2 #t)) (lambda () (set-onoff doorff2 #f)))


; I guess I'd better be nice and give 'em a save point here. Aren't I nice? Well they earned it.
(diamond 243 190)

; Groovy moving platforms. Sheesh, I'd hate to do this in the dark.
(add-cyclic-platform 249 249 249 249 -3. 3. 6. .5)
(add-cyclic-platform 249 248 249 248 -4. 1. 4. .5)
(add-cyclic-platform 249 247 249 247 -5. 0. 6. .5)
(add-cyclic-platform 249 246 249 246 -5. -2. 4. .5)
(add-cyclic-platform 249 245 249 245 -8. -4. 6. .5)

; For the Homer Simpsons out ther, a sign to warn that icy pond will kill the marble.
(set-primary-color (sign (_ "Danger!") 1.0 15.0 0.0 231 180) 0.8 0.8 0.4)

; Not just one, but a whole bunch of Mr Black's! Muhahahahahah!!!!!
(define mr-black1 (new-mr-black 196 236))
(define mr-black2 (new-mr-black 199 252))
(define mr-black3 (new-mr-black 196 252))
(define mr-black4 (new-mr-black 199 236))
(define mr-black5 (new-mr-black 196 243))
(define mr-black6 (new-mr-black 196 226))
(define mr-black7 (new-mr-black 199 226))
(define mr-black8 (new-mr-black 196 219))
(define mr-black9 (new-mr-black 199 219))

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
         (add-modpill 190 252 *mod-spike* 40 40)
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
(switch 225 235 (lambda () #f)  (lambda () (if invisible (begin (set! invisible #f) (set! mr-black1 (new-mr-black 224 249))(set-speed spike 0.2)))) )
(define spike (add-spike 250 218 speed -0.20))

; Shute leading you outside
(define pipe-shute (pipe 250 217 2.4 250 205.5 -6.0 0.7) )
(set-wind pipe-shute 2.0 2.0)



; Spikes to help avoid any Mr Blacks that just won't go away.
(add-modpill 253 203 *mod-spike* 25 40)

; A bunch of pretty waving flags to distract the player and eat up time.
(add-flag 240 245 200 #t 0.1)
(add-flag 248 229 250 #t 0.1)
(add-flag 214 204 150 #t 0.1)
(add-flag 246 195 750 #t 0.1) ; Ok, I guess this one is worth getting though.
(set-primary-color (sign (_ "Eternity") 1.0 15.0 0.0 248 195) 0.8 0.4 0.8)
(add-flag 238 180 150 #t 0.1)
(add-flag 222 180 250 #t 0.1)
(add-flag 189 191 150 #t 0.1)
(add-flag 201 216 150 #t 0.1)
(add-flag 189 242 250 #t 0.1)
(add-flag 196 252  25 #t 0.1) ; Hmmmm, why did I bother? Sucker! :p
(add-flag 228 251 150 #t 0.1)
(add-flag 228 251 150 #t 0.1)
(add-flag 247 214 500 #t 0.1) ; Oh ok, grab this one also - if you don't burn to many calories to get it.

; Secret level....ssshhhh!!!!!!!
(define pipe-secret(pipe 253.0 169.8 -9.2 252.8 155.5 -8.9 0.7))
(set-wind pipe-secret 5.0 5.0)
(add-flag 253 149 250 #t 0.1)
(add-flag 253 147 250 #t 0.1)
(add-flag 253 147 250 #t 0.1)
(add-flag 249 144 250 #t 0.1)
(add-flag 249 149 250 #t 0.1)
(add-flag 249 154 250 #t 0.1)
(add-flag 247 149 250 #t 0.1)
(add-flag 244 149 250 #t 0.1)
(add-flag 244 154 250 #t 0.1)



; "For the wages of sin is death, but the gift of God is eternal
;  life in Jesus Christ our Lord" - Romans 6:23
