;;; lv8 - crazy factory

(fog) 
;; Add some flashing fog
(fog-color 0.2 0.2 0.2)
(animator 5.0 0.0 1.0 0.2 0.5 *animator-bounce*
		  (lambda (v) (fog-color v (+ 0.1 (* 0.5 v)) 0.2)))
(set-track-name "Crazy Factory")
(set-author "Mathias Broxvall")

(start-time 150) 
(set-start-position 244.5 244.5)
;(set-start-position 225.5 232.5)
(map-is-transparent #t)

(add-flag 244 231 250 1 0.1)
(add-flag 236 229 100 1 0.1)
(add-flag 225 231 100 1 0.1)
(diamond 218.5 232.5)

(switch 233.5 229.5
		(lambda()(set-cell-velocity 241 238 234 238 1.0 0.0))
		(lambda()(set-cell-velocity 241 238 234 238 -1.0 0.0)))
(smart-trigger 228.5 238.5 4.0 
			   (lambda()(camera-angle 0.0 1.5)) 
			   (lambda()(camera-angle 0.0 0.0)))
(add-cyclic-platform 
 224 237 226 239 -2.0 +1.0 0.0 0.80)

(add-modpill 229.5 228.5 *mod-extra-life* 30 -1)
(define door0animator
  (animator 2.0 0.0 0.0 -4.5 -6.0 *animator-stop*
			(lambda (v) (set-cell-heights 230 231 230 231 -6.0 -6.0 v v) (set-cell-heights 231 231 231 231 v v -6.0 -6.0))))
(switch 225.5 227.5
		(lambda() (set-animator-direction door0animator -1.0))
		(lambda() (set-animator-direction door0animator 1.0)))
(define mr0 (new-mr-black 219.5 232.5))
(set-texture mr0 "track.png")
(set-primary-color mr0 1.0 0.2 0.2) 
(if (= (difficulty) *hard*)
	(set-modtime mr0 *mod-spike* -1.))

(define mr1 (new-mr-black 214.5 232.5))
(set-texture mr1 "track.png")
(set-primary-color mr1 0.2 1.0 0.2) 
(if (>= (difficulty) *normal*)
	(set-modtime mr1 *mod-spike* -1.))

(define mr2 (new-mr-black 214.5 227.5))
(set-texture mr2 "track.png")
(set-primary-color mr2 0.5 0.5 1.0) 
(if (= (difficulty) *hard*)
	(set-modtime mr2 *mod-spike* -1.))

(add-goal 210 227 #t "lv9")
