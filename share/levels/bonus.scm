;;; bonus
;; Define the variable bonus-goto before coming here

(night)
(add-goal 232 241 #t bonus-goto) 
(set-bonus-level bonus-goto)
(set-start-position 240 241)
(if (= (difficulty) *easy*)
	(start-time 90) 
	(if (= (difficulty) *normal)
		(start-time 75)
		(start-time 60)))

;; It costs 200 to enter the bonus level
(add-flag 240 241 -200 0 1.0)

(add-flag 232 250 20 1 0.1) (add-flag 232 249 20 1 0.1) (add-flag 232 248 20 1 0.1) 
(add-flag 231 250 20 1 0.1) (add-flag 231 249 20 1 0.1) (add-flag 231 248 20 1 0.1) 
(add-flag 230 250 20 1 0.1) (add-flag 230 249 20 1 0.1) (add-flag 230 248 20 1 0.1) 

(add-flag 249 250 20 1 0.1) (add-flag 249 249 20 1 0.1) (add-flag 249 248 20 1 0.1)
(add-flag 248 250 20 1 0.1) (add-flag 248 249 20 1 0.1) (add-flag 248 248 20 1 0.1)
(add-flag 247 250 20 1 0.1) (add-flag 247 249 20 1 0.1 )(add-flag 247 248 20 1 0.1)

(add-flag 250 242 20 1 0.1) (add-flag 250 241 20 1 0.1) (add-flag 250 240 20 1 0.1)
(add-flag 249 242 20 1 0.1) (add-flag 249 241 20 1 0.1) (add-flag 249 240 20 1 0.1)
(add-flag 248 242 20 1 0.1) (add-flag 248 241 20 1 0.1) (add-flag 248 240 20 1 0.1)

(add-flag 248 235 20 1 0.1) (add-flag 248 234 20 1 0.1) (add-flag 248 233 20 1 0.1)
(add-flag 247 235 20 1 0.1) (add-flag 247 234 20 1 0.1) (add-flag 247 233 20 1 0.1)
(add-flag 246 235 20 1 0.1) (add-flag 246 234 20 1 0.1) (add-flag 246 233 20 1 0.1)

(add-flag 241 235 20 1 0.1) (add-flag 241 234 20 1 0.1) (add-flag 241 233 20 1 0.1)
(add-flag 240 235 20 1 0.1) (add-flag 240 234 20 1 0.1) (add-flag 240 233 20 1 0.1)
(add-flag 239 235 20 1 0.1) (add-flag 239 234 20 1 0.1) (add-flag 239 233 20 1 0.1)

(add-modpill 227.5 243.5 *mod-spike* 120 0) 
(add-flag 227 243 -250 0 0.5)
(sign "Spikes, 400pt" 1.0 0.0 0.0 227.5 243.5)

(add-modpill 225.5 243.5 *mod-speed* 120 0) 
(add-flag 225 243 -250 0 0.5)
(sign "Speed, 300pt" 1.0 0.0 0.0 225.5 243.5)

(add-modpill 223.5 243.5 *mod-jump* 120 0) 
(add-flag 223 243 -250 0 0.5)
(sign "Jump, 200pt" 1.0 0.0 0.0 223.5 243.5)

;; modpills: 227 243 .. 224 243
