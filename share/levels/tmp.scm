;;; tmp

(day)
(set-track-name "Acid")
(set-author "Mathias Broxvall")
(map-is-transparent #t)

(start-time 150) 
(set-start-position 250.5 252.5)

(define f0 (fountain 245.5 240.5 -8.5 0.03 0.0 200.0))
(define waterAnimator (animator 1.0 0.0 0.0 -8.6 -8.1 *animator-stop* 
								(lambda (v) (set-cell-water-heights 246 241 243 238 v v v v))))
			
(add-modpill 249.5 252.5 *mod-float* 60 -1)

(switch 245 243 
		(lambda () 
		  (set-animator-direction waterAnimator 0.2)
		  (set-fountain-strength f0 200.0)
		  ) 
		(lambda() 
		  (set-animator-direction waterAnimator -0.2)
		  (set-fountain-strength f0 0.0)
		))


(add-cactus 237 237 0.3)
(add-teleport 235 235 236 236 0.3)
(add-modpill 234.5 234.5 *mod-extra-life* 60 -1)
(add-bird 230 230 232 222 0.5 0.9)
(define f0 (fountain 231.5 231.5 -8.5 0.03 0.0 200.0))
