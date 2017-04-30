;;; Clowning Around
;;; a kids level that is semi-hard.

(set-track-name "Clowning Around")
(set-author "Dietrich Radel")

(set-start-position 234.5 232.5)
(add-goal 221 226 #t "ki5")

;;; how much time each difficulty level gets
(cond
 ((= (difficulty) *easy*)(begin (start-time 180) (restart-time 60)))
 ((= (difficulty) *normal*)(begin (start-time 150) (restart-time 45)))
 ((= (difficulty) *hard*)(begin (start-time 120) (restart-time 30)))
)


(add-flag 243 237 100 1 0.1)
(add-flag 245 239 100 1 0.1)
(add-flag 225 232 150 1 0.1)
(add-flag 226 227 150 1 0.1)

(add-modpill 223.5 231.5 *mod-spike* 25 40)

;; This would create a forcefield and a switch turning it on/off
(define ff (forcefield 227.0 226.0 0.0 1.0 0.0 0.0 1.0 *ff-bounce*))
(switch 223.0 230.0 (lambda () (set-onoff ff #t)) (lambda () (set-onoff ff #f)))
; Savepoint for everyone
(diamond 230 239)

; Savepoints for easy only
(if (= (difficulty) *easy*) 
	(begin (diamond 242.5 234.5)
		   (diamond 235.5 232.5)))


