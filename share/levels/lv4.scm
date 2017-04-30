;;; lv4

(day)
(set-track-name "Labyrinth")
(set-author "Mathias Broxvall")
(jump 0.8) ;; To avoid cheating
(set-start-position 251.5 251.5)
(start-time (- 180 (* 10 (difficulty))))

(add-modpill 222.5 237.5 *mod-extra-life* 30 -1)


(diamond 232.5 240.5)

(define ff1 (forcefield 233.0 247.5 0.0 3.0 0.0 0.0 0.8 *ff-bounce*))
(define ff3 (forcefield 247.0 243.0 0.0 0.0 1.0 0.0 0.8 *ff-bounce*))
(switch 248.5 248.5 (lambda()(set-onoff ff1 #t)) (lambda()(set-onoff ff1 #f)))
(switch 234.5 252.5 (lambda()(set-onoff ff3 #t)) (lambda()(set-onoff ff3 #f)))
(add-flag 233 252 250 1 0.1)
(add-modpill 235.5 252.5 *mod-spike* 30 60)
(add-flag 243 252 150 1 0.1) (add-flag 234 224 150 1 0.1)
(define mr-black (new-mr-black 235.5 245.8))
(cond
 ((= (difficulty) *easy*) (set-acceleration mr-black 2.0))
 ((= (difficulty) *normal*) (set-acceleration mr-black 3.0))
 ((= (difficulty) *hard*) (set-acceleration mr-black 4.0)))

(define ff2 (forcefield 230.0 219.5 0.0 1.0 0.0 0.0 0.8 *ff-bounce*))
(switch 222.5 241.5 (lambda()(set-onoff ff2 #t))(lambda()(set-onoff ff2 #f)))

(add-goal 212 219 #t "lv5")

;; new
(add-heightmodifier 10 222 219 -4.2 -2.0 0.2 0.0)
(add-heightmodifier 11 222 219 -4.2 -2.0 0.2 0.0)

(add-heightmodifier 10 221 219 -4.2 -2.0 0.2 0.6)
(add-heightmodifier 11 221 219 -4.2 -2.0 0.2 0.7)

(add-heightmodifier 10 220 219 -4.2 -2.0 0.2 1.2)
(add-heightmodifier 11 220 219 -4.2 -2.0 0.2 1.3)

(add-heightmodifier 10 219 219 -4.2 -2.0 0.2 1.8)
(add-heightmodifier 11 219 219 -4.2 -2.0 0.2 1.9)

(add-heightmodifier 10 218 219 -4.2 -2.0 0.2 2.4)
(add-heightmodifier 11 218 219 -4.2 -2.0 0.2 2.5)

(add-heightmodifier 10 217 219 -4.2 -2.0 0.2 3.0)
(add-heightmodifier 11 217 219 -4.2 -2.0 0.2 3.1)

(add-heightmodifier 10 216 219 -4.2 -2.0 0.2 3.6)
(add-heightmodifier 11 216 219 -4.2 -2.0 0.2 3.7)

(add-heightmodifier 10 215 219 -4.2 -2.0 0.2 4.2)
(add-heightmodifier 11 215 219 -4.2 -2.0 0.2 4.3)

(add-heightmodifier 10 214 219 -4.2 -2.0 0.2 4.8)
(add-heightmodifier 11 214 219 -4.2 -2.0 0.2 4.9)

