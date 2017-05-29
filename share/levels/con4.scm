;;; Scary track

(day)
(set-track-name (_ "Scary"))
(set-start-position 253.5 251.5)
(add-goal 253 215 #t "con5")
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
   (add-flag 226 242 100 1 0.1)
   (trigger 226 242 0.5 (lambda () (set-start-position 226.5 242.5)))
   (trigger 227 244 0.5 (lambda () (set-start-position 227.5 244.5)))
 )
)

(add-flag 246 224 100 1 0.1)

;;; spikes and save point for last stetch
(trigger 252 243 0.5 (lambda () (set-start-position 252.5 243.5)))
(add-modpill 252.0 243.0 *mod-spike* 25 40)


;;; mr black for upstairs
(set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 227 244) 0.8 0.8 0.4)
(define mr-black (new-mr-black 227 244))


;;; position the pill in relation to difficulty setting
;;; none for 'hard'
(cond
 ((= (difficulty) *easy*) (add-modpill 244.5 241.5 *mod-spike* 25 40))
 ((= (difficulty) *normal*) (add-modpill 244.0 241.8 *mod-spike* 25 40))
)


;;; speed of the spikes depends on difficulty level
(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.60))
)
;;; Moving spikes
(add-spike 242.5 245.5 speed -0.00)
(add-spike 243.5 245.5 speed -0.25)
 


;;; Mr Black for the lower level.
(define mr-black (cond
 ((= (difficulty) *normal*)
   (set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 247 237) 0.8 0.8 0.4)
   (new-mr-black 247 237)
 )
 ((= (difficulty) *hard*)
   (set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 247 237) 0.8 0.8 0.4)
   (new-mr-black 247 237)
 )
))

 


;; This would create a forcefield and a switch turning it on/off
(define ff (forcefield 253.0 238.5 0.0 1.0 0.0 0.0 1.5 *ff-bounce*))
(switch 244.5 236.8 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
