;; SIX LEVELS
;; No jumping!

;;  This level is filled with different sorts of obstacles, for example the cannon which shoots spiky marbles at you.



(day)
(set-track-name (_ "No jumping!"))
(set-author "Ulrik Enstad")
(cond
 ((= (difficulty) *easy*)
         (start-time 180)
 )
 ((= (difficulty) *normal*)
         (start-time 150)
 )
 ((= (difficulty) *hard*)
         (start-time 100)
 )
)
(set-start-position 250.5 250.5)
(jump 0.0)
(restart-time 0)

;; sign
(sign (_ "No jumping!") 1 20.0 -1 249.5 249.5)

;; the cannon
(set-wind (pipe 251.7 243.5 4.8 249.0 243.5 4.8 0.3) 16.0 0.0)

;; when you roll over these spots, the cannon will get ammo! Be careful!
(smart-trigger 248.5 248.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 248.5 243.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 245.5 244.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 245.5 241.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 245.5 248.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 245.5 245.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 242.5 243.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

(smart-trigger 243.5 242.5 0.5 (lambda ()
(set-primary-color (set-modtime (set-modtime (new-mr-black 252.5 243.5) *mod-small* -1.) *mod-spike* -1.) 1.0 0.0 0.0))
(lambda () (day)))

;; some delicious flags
(add-flag 243 248 50 1 0.1)
(set-primary-color (add-flag 233 202 500 1 0.1) 1.0 1.0 0.0)
(set-primary-color (add-flag 246 222 500 1 0.1) 1.0 1.0 0.0)
(add-flag 245 233 50 1 0.1)
(define timeflag1 (add-flag 229 215 0 1 0.1))
(time-on-death timeflag1 30)
(set-primary-color timeflag1 0.0 1.0 0.0)
(add-modpill 231.5 202.5 *mod-extra-life* 10 0)

;; spikes on the four-colored field
(define spike1 (add-spike 234.5 216.5 .2 0.00))
(define spike2 (add-spike 234.5 215.5 .2 0.00))
(define spike3 (add-spike 234.5 218.5 .2 0.20))
(define spike4 (add-spike 234.5 217.5 .2 0.20))

(define spike5 (add-spike 233.5 216.5 .2 0.00))
(define spike6 (add-spike 233.5 215.5 .2 0.00))
(define spike7 (add-spike 233.5 218.5 .2 0.20))
(define spike8 (add-spike 233.5 217.5 .2 0.20))

(define spike9 (add-spike 232.5 218.5 .2 0.40))
(define spike10 (add-spike 232.5 217.5 .2 0.40))
(define spike11 (add-spike 232.5 216.5 .2 0.60))
(define spike12 (add-spike 232.5 215.5 .2 0.60))

(define spike13 (add-spike 231.5 218.5 .2 0.40))
(define spike14 (add-spike 231.5 217.5 .2 0.40))
(define spike15 (add-spike 231.5 216.5 .2 0.60))
(define spike16 (add-spike 231.5 215.5 .2 0.60))

(cond
 ((= (difficulty) *normal*)
(set-speed spike1 0.2)
(set-speed spike2 0.2)
(set-speed spike3 0.2)
(set-speed spike4 0.2)

(set-speed spike5 0.2)
(set-speed spike6 0.2)
(set-speed spike7 0.2)
(set-speed spike8 0.2)

(set-speed spike9 0.2)
(set-speed spike10 0.2)
(set-speed spike11 0.2)
(set-speed spike12 0.2)

(set-speed spike13 0.2)
(set-speed spike14 0.2)
(set-speed spike15 0.2)
(set-speed spike16 0.2)
 )
 ((= (difficulty) *easy*)
(set-speed spike1 0.1)
(set-speed spike2 0.1)
(set-speed spike3 0.1)
(set-speed spike4 0.1)

(set-speed spike5 0.1)
(set-speed spike6 0.1)
(set-speed spike7 0.1)
(set-speed spike8 0.1)

(set-speed spike9 0.1)
(set-speed spike10 0.1)
(set-speed spike11 0.1)
(set-speed spike12 0.1)

(set-speed spike13 0.1)
(set-speed spike14 0.1)
(set-speed spike15 0.1)
(set-speed spike16 0.1)
 )
 ((= (difficulty) *hard*)
(set-speed spike1 0.5)
(set-speed spike2 0.5)
(set-speed spike3 0.5)
(set-speed spike4 0.5)

(set-speed spike5 0.5)
(set-speed spike6 0.5)
(set-speed spike7 0.5)
(set-speed spike8 0.5)

(set-speed spike9 0.5)
(set-speed spike10 0.5)
(set-speed spike11 0.5)
(set-speed spike12 0.5)

(set-speed spike13 0.5)
(set-speed spike14 0.5)
(set-speed spike15 0.5)
(set-speed spike16 0.5)
 )
)

;; roll over this spot to higher the four-colored field
(define blinker (add-colormodifier 3 226 221 0.5 1.0 0.5 1.0))
(trigger 226.5 221.5 0.5 (lambda ()
(set-cell-heights 231 215 234 218 -0.8 -0.8 -0.8 -0.8)
(set-onoff blinker #f)
))

(smart-trigger 226.5 221.5 0.5 (lambda () (play-effect "click.wav")) (lambda () (day)))

;; hmmm
(diamond 236 218)

;; goal
(add-goal 226 225 #t "sl2")


;; hints to win level:
;; in the cannon maze, choose the way farest from the cannon.
;; move between the spikes where it's impossible to be hit.







