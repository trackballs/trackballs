;;; Clowning Around
;;; a kids level that is semi-hard.

(set-track-name (_ "Clowning Around"))
(set-author "Dietrich Radel")

(set-start-position 234 232)
(add-goal 221 226 #t "ki5")

;;; how much time each difficulty level gets
(cond
 ((= (difficulty) *easy*)(begin (start-time 180) (restart-time 60)))
 ((= (difficulty) *normal*)(begin (start-time 150) (restart-time 45)))
 ((= (difficulty) *hard*)(begin (start-time 120) (restart-time 30)))
)


(add-flag 243 237 100 #t 0.1)
(add-flag 245 239 100 #t 0.1)
(add-flag 225 232 150 #t 0.1)
(add-flag 226 227 150 #t 0.1)

(add-modpill 223 231 *mod-spike* 25 40)

;; This would create a forcefield and a switch turning it on/off
(define ff (forcefield 226.5 225.5 0.0 1.0 0.0 0.0 1.0 *ff-bounce*))
(switch 222.5 229.5 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
; Savepoint for everyone
(diamond 229.5 238.5)

; Savepoints for easy only
(if (= (difficulty) *easy*) 
  (begin (diamond 242 234)
       (diamond 235 232)))


