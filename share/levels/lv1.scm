;;; level 1
;; Note. This level is much more complex than the others since
;; I want to demonstrate the flexibility of what can be done with
;; the GUILE scripting language. If you are looking to create a level
;; yourself it is better to look at one of the other levels first and when
;; you have gotten started you might look here for some ideas.

(day)
(set-track-name (_ "Warmup"))
(set-author "Mathias Broxvall")
(fog)

(restart-time 60.0)

(set-start-position 252 252)
(start-time 150)

(snow 0.5)
;(add-goal 252 250 #f "lv2") for debugging
;(add-modpill 252 253 *mod-large* 30 -1)
;(add-modpill 253 253 *mod-small* 30 -1)

(if (= (difficulty) *easy*) (add-modpill 252 253 *mod-spike* 30 -1))

;; Secret bonus level
(define bonus-goto "lv2")
(add-goal 253 203 #f "bonus")
(add-modpill 253 196 *mod-frozen* 5 (- 22 (* 3 (difficulty))))

;; This would create a forcefield and a switch turning it on/off
(if (= (difficulty) *hard*)
  (let ()
    (define ff (forcefield 251.0 250 0.0 
               2.0 0.0 0.0 
               0.5 *ff-kill*))
    (switch 252 249 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
    ))

;; This is the goal
(add-goal 219 204 #f "lv2")

(add-flag 252 232 250 #f 1.0) ;; After the jump
(add-flag 241 245 150 #t 0.1) ;; Where the black ball is
(add-flag 231 229 100 #f 1.0)
(add-flag 224 212 100 #t 0.1)
(add-flag 219 199 150 #t 0.1)
(add-flag 224 198 250 #t 0.1)

(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.10))
 ((= (difficulty) *normal*) (set! speed 0.20))
 ((= (difficulty) *hard*) (set! speed 0.30))
)

(add-spike 250 228 speed -0.00)
(add-spike 249 228 speed -0.20)
(add-spike 248 228 speed -0.40)
(add-spike 247 228 speed -0.60)
(add-spike 246 228 speed -0.80)
(add-spike 245 228 speed -1.00)

(add-flag 240 219 150 #t 0.1)
(add-flag 232 218 250 #t 0.1)


; new
(add-sidespike 245 245 speed 0.0 4)
(add-sidespike 247 245 speed 0.1 4)
(add-sidespike 249 245 speed 0.2 4)

(set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 241 245) 0.8 0.8 0.4)
(define mr-black (new-mr-black 241 245))
(if (= (difficulty) *hard*) (set-modtime mr-black *mod-spike* -1.))
(set-acceleration mr-black 2.0)

(define *min-value* 0.0)
(define *max-value* 1.0)
(define *starting-direction* 1.0)

(define mr-black-animator
  (animator
     1.0 0.0 *starting-direction* *min-value* *max-value* *animator-bounce*
     (lambda (value) (set-primary-color mr-black value value 0.0))
     )
  )


;(set-onoff mr-black #f)
;(set-modtime mr-black *mod-spike* -1.)
;(set-primary-color mr-black 0.5 1.0 0.5)
;(set-specular-color mr-black 0.5 1.0 0.5)

(set-primary-color (sign (_ "and his Minions") 1.0 15.0 0.0 233 245) 0.8 0.8 0.4)
(new-baby 233 245)
(new-baby 231.7 244.5)
(new-baby 233.0 245.5)
(new-baby 231.7 244.2)
(new-baby 232.3 246.0)
(new-baby 233.4 245.7)

(cond
 ((= (difficulty) *easy*) (set! speed 0.60))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.20))
)

(define plat1 (add-cyclic-platform 
 ;;; Positions
         238 228 239 229
 ;;; Low, High, Time Offset, Speed
 3. 4. 0. speed)
)

(add-cyclic-platform 
 ;;; Positions
 236 228 237 229
 ;;; Low, High, Time Offset, Speed
 2. 4. 0.5 (+ 0.10 speed))

(add-cyclic-platform 
 ;;; Positions
 234 228 235 229
 ;;; Low, High, Time Offset, Speed
 3. 4. 0. speed)

(add-cyclic-platform 
 ;;; Positions
 230 225 231 229
 ;;; Low, High, Time Offset, Speed
 2. 3. 0.5 speed)

(diamond 241.5 228.5)

;;; modpill-types:
;;; *mod-speed* - "Speedball": Makes the player go faster
;;; *mod-jump* - "Jumpjets": Makes the player jump higher
;;; *mod-spike* - "Spikes": Damages other balls more in collisions. Increases friction on ice
;;; *mod-glass* - "Glassball": Makes ball more sensitive to crashes
;;; *mod-dizzy* - "Dizzy": Makes ball dizzy 
;;; *mod-frozen* - "Frozen": Player can't move ball as long as it is frozen

(add-modpill 231 212 *mod-spike* 25 40)

;;;;;;;;;;;;;;;;;;;;;;;;,
;;; Note. These are some extra functions you can use

;; Calls your lambda expression when player is close to X,Y
;(trigger X Y R (lambda () <something to do>))

(sign "debug test" 1.0 15.0 0.0 10.5 10.5)
