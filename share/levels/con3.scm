; hell-level

(day)
(jump 0.7) ;; MB to fix a possible cheat
(fog)
(fog-color .0 .4 .4)

(set-track-name "Hell")
(set-author "Wouter Tellier")
(add-goal 246 248 #t "con4")

(start-time 150)
(set-start-position 252.5 251.5)


; To castle gate
(add-cyclic-platform 231 235 231 235 -7. -2. 4. .5)

; Fire
(add-cyclic-platform 238 216 238 216 -8. -6. 6. .5)
(add-cyclic-platform 239 216 239 216 -6. -3. 4. .5)
(add-cyclic-platform 239 217 239 217 -3. -2. 6. .5)
(add-cyclic-platform 240 217 240 217 -2. -1. 4. .5)
(add-cyclic-platform 240 218 240 218 -1. 0. 6. 0.5)
(add-cyclic-platform 241 218 241 218 0. -1. 4. 0.5)
(add-cyclic-platform 242 219 242 219 0. -3. 4. 0.5)
(add-cyclic-platform 243 219 243 219 -3. -4. 6. 0.5)
(add-cyclic-platform 243 218 243 218 -4. -5. 4. 0.5)
(add-cyclic-platform 244 218 244 218 -5. -6. 6. 0.5)
(add-cyclic-platform 245 218 245 218 -6. -7. 4. 0.5)
(add-cyclic-platform 246 218 246 218 -7. -8. 6. 0.5)

; bit up after the fire
(add-cyclic-platform 247 218 247 218 -8. -5.5 6. 0.5)

; spikes for making the shortcut more difficult
(add-spike 247.5 227.5 .75 1.)
(add-spike 252.5 227.5 .75 1.)

; Speed so we can make the jump from the castle
(add-modpill 228 234 *mod-speed* 10 20)

; Hells gate keeping mr black safe ...
(define hellgate (forcefield 241.0 234.5 0.0 5.0 0.0 0.0 1.0 *ff-bounce*))
; ... until the switch is turned
(switch 243 235.5 (lambda() (set-onoff hellgate #f)) (lambda() (set-onoff hellgate #f))) 

(define satan (new-mr-black 243 233))
(set-horizon satan 100)

(sign "Hells entrance" 1 30 -1 243 246)
(sign "Hells gate" 1 30 -1 243 234)
(sign "Hells castle" 1 30 -1 228 235)
(sign "Hells fire" 1 30 -1 238 216)
(sign "Hell freezing over" 1 30 -1 253 242)
