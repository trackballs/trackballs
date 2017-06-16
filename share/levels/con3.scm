; hell-level

(day)
(jump 0.7) ;; MB to fix a possible cheat
(fog)
(fog-color .0 .4 .4)

(set-track-name (_ "Hell"))
(set-author "Wouter Tellier")
(add-goal 245 247 #t "con4")

(start-time 150)
(set-start-position 251 250)


; To castle gate
(add-cyclic-platform 230 234 230 234 -7. -2. 4. .5)

; Fire
(add-cyclic-platform 237 215 237 215 -8. -6. 6. .5)
(add-cyclic-platform 238 215 238 215 -6. -3. 4. .5)
(add-cyclic-platform 238 216 238 216 -3. -2. 6. .5)
(add-cyclic-platform 239 216 239 216 -2. -1. 4. .5)
(add-cyclic-platform 239 217 239 217 -1. 0. 6. 0.5)
(add-cyclic-platform 240 217 240 217 0. -1. 4. 0.5)
(add-cyclic-platform 241 218 241 218 0. -3. 4. 0.5)
(add-cyclic-platform 242 218 242 218 -3. -4. 6. 0.5)
(add-cyclic-platform 242 217 242 217 -4. -5. 4. 0.5)
(add-cyclic-platform 243 217 243 217 -5. -6. 6. 0.5)
(add-cyclic-platform 244 217 244 217 -6. -7. 4. 0.5)
(add-cyclic-platform 245 217 245 217 -7. -8. 6. 0.5)

; bit up after the fire
(add-cyclic-platform 246 217 246 217 -8. -5.5 6. 0.5)

; spikes for making the shortcut more difficult
(add-spike 246 226 .75 1.)
(add-spike 251 226 .75 1.)

; Speed so we can make the jump from the castle
(add-modpill 226.5 232.5 *mod-speed* 10 20)

; Hells gate keeping mr black safe ...
(define hellgate (forcefield 239.5 233.0 0.0 5.0 0.0 0.0 1.0 *ff-bounce*))
; ... until the switch is turned
(switch 241 234 (lambda() (set-onoff hellgate #f)) (lambda() (set-onoff hellgate #f))) 

(define satan (new-mr-black 241.5 231.5))
(set-horizon satan 100)

(sign (_ "Hell's entrance") 1 30 -1 241.5 244.5)
(sign (_ "Hell's gate") 1 30 -1 241.5 232.5)
(sign (_ "Hell's castle") 1 30 -1 226.5 233.5)
(sign (_ "Hell's fire") 1 30 -1 236.5 214.5)
(sign (_ "Hell freezing over") 1 30 -1 251.5 240.5)
