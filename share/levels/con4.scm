;;; Scary track

(day)
(set-track-name (_ "Scary"))
(set-start-position 253 251)
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
   (add-flag 226 242 100 #t 0.1)
   (trigger 225.5 241.5 0.5 (lambda () (set-start-position 226 242)))
   (trigger 226.5 243.5 0.5 (lambda () (set-start-position 227 244)))
 )
)

(add-flag 246 224 100 #t 0.1)

;;; spikes and save point for last stetch
(trigger 251.5 242.5 0.5 (lambda () (set-start-position 251.5 243.5)))
(add-modpill 251.5 242.5 *mod-spike* 25 40)


;;; mr black for upstairs
(set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 226.5 243.5) 0.8 0.8 0.4)
(define mr-black (new-mr-black 226.5 243.5))


;;; position the pill in relation to difficulty setting
;;; none for 'hard'
(cond
 ((= (difficulty) *easy*) (add-modpill 244 241 *mod-spike* 25 40))
 ((= (difficulty) *normal*) (add-modpill 243.5 241.3 *mod-spike* 25 40))
)


;;; speed of the spikes depends on difficulty level
(define speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! speed 0.20))
 ((= (difficulty) *normal*) (set! speed 0.40))
 ((= (difficulty) *hard*) (set! speed 0.60))
)
;;; Moving spikes
(add-spike 242 245 speed -0.00)
(add-spike 243 245 speed -0.25)
 


;;; Mr Black for the lower level.
(define mr-black (cond
 ((= (difficulty) *normal*)
   (set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 246.5 236.5) 0.8 0.8 0.4)
   (new-mr-black 246.5 236.5)
 )
 ((= (difficulty) *hard*)
   (set-primary-color (sign (_ "Mr. Black") 1.0 15.0 0.0 246.5 236.5) 0.8 0.8 0.4)
   (new-mr-black 246.5 236.5)
 )
))

 


;; This would create a forcefield and a switch turning it on/off
(define ff (forcefield 252.5 238.0 0.0 1.0 0.0 0.0 1.5 *ff-bounce*))
(switch 244.0 236.3 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
