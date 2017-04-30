(set-track-name "High Nausea Rating")
(set-author "Ulrik Enstad")
(set-start-position 238.5 238.5)

(set-acceleration (set-primary-color (new-mr-black 225.5 221.5) 0.5 0.5 0.5) 1.6)

(pipe 224.5 208.5 -0.9 225.5 208.5 -1.3 0.4)
(pipe-connector 225.5 208.5 -1.3 0.4)
(pipe 225.5 208.5 -1.3 225.4 208.5 -3.1 0.4)
(pipe-connector 225.4 208.5 -3.1 0.4)
(pipe 225.4 208.5 -3.1 222.5 208.5 -3.1 0.4)

(diamond 219 207)

(add-goal 226 184 #f "Shop2")
(add-goal 227 184 #f "Shop2")

(add-flag 210 197 30 0 1.5)
(add-flag 222 208 30 0 0.5)
(add-flag 233 233 50 0 0.5)

(define bonus-goto "Shop2")
(set-primary-color (add-goal 217 218 #t "bonusb") 1.0 1.0 1.0)

(fog)
(fog-color 0.8 0.3 0.0)

(jump 0.0)

(cond
 ((= (difficulty) *easy*)
         (start-time 100)
 )
 ((= (difficulty) *normal*)
         (start-time 90)
 )
 ((= (difficulty) *hard*)
         (start-time 70)
 )
)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (set-modtime
(new-mr-black 227.5 216.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
 ((= (difficulty) *normal*)
         (set-primary-color (set-acceleration (set-modtime
(new-mr-black 227.5 216.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
 ((= (difficulty) *hard*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color (new-mr-black 227.5 216.5) 1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)

 )
)

(restart-time 0)

