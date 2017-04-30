;;; Scary track

(day)
(set-track-name "Scary")
(set-start-position 254.5 252.5)
(add-goal 254 216 #t "")
(set-author "Dietrich Radel")

;;; how much time each difficulty level gets
(cond
 ((= (difficulty) *easy*)(start-time 175)) 
 ((= (difficulty) *normal*)(start-time 160))
 ((= (difficulty) *hard*)(start-time 150))
)


;;; checkpoint for easy mode
(cond
 ((= (difficulty) *easy*)
   (add-flag 227 243 100 1 0.1)
   (trigger 227 243 0.5 (lambda () (set-start-position 227.5 243.5)))
   (trigger 228 245 0.5 (lambda () (set-start-position 228.5 245.5)))
 )
)

(add-flag 247 225 100 1 0.1)

;;; spikes and save point for last stetch
(trigger 253 244 0.5 (lambda () (set-start-position 253.5 244.5)))
(add-modpill 253.0 244.0 *mod-spike* 25 40)


;;; mr black for upstairs
(set-primary-color (sign "Mr. Black" 1.0 15.0 0.0 228 245) 0.8 0.8 0.4)
(define mr-black (new-mr-black 228 245))


;;; position the pill in relation to difficulty setting
;;; none for 'hard'
(cond
 ((= (difficulty) *easy*) (add-modpill 245.5 240.5 *mod-spike* 25 40))
 ((= (difficulty) *normal*) (add-modpill 245.0 240.8 *mod-spike* 25 40))
)


;;; speed of the spikes depends on difficulty level
(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.60))
)
;;; Moving spikes
(add-spike 243.5 246.5 speed -0.00)
(add-spike 244.5 246.5 speed -0.25)
 


;;; Mr Black for the lower level.
(cond
 ((= (difficulty) *normal*)
 (set-primary-color (sign "Mr. Black" 1.0 15.0 0.0 248 238) 0.8 0.8 0.4)
 (define mr-black (new-mr-black 248 238))
 )
 ((= (difficulty) *hard*)
 (set-primary-color (sign "Mr. Black" 1.0 15.0 0.0 248 238) 0.8 0.8 0.4)
 (define mr-black (new-mr-black 248 238))
 )
)

 


;; This would create a forcefield and a switch turning it on/off
(define ff (forcefield 254.0 239.5 0.0 1.0 0.0 0.0 1.5 *ff-bounce*))
(switch 245.5 237.8 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
