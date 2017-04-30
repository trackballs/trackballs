(set-track-name "Towers of Heaven")
(set-author "Ulrik Enstad")
(set-start-position 248.5 247.5)
(map-is-transparent #t)
(restart-time 0)

(jump 0.0)

(add-cyclic-platform 235 236 237 237 3.1 4.6 0.00 .3)
(add-cyclic-platform 235 234 237 235 3.1 4.6 1.00 .3)

(pipe 237.5 228.5 3.6 238.5 228.5 3.6 0.4)
(pipe-connector 238.5 228.5 3.6 0.4)
(pipe 238.5 228.5 3.6 239.5 228.5 2.7 0.4)
(pipe-connector 239.5 228.5 2.7 0.4)
(pipe 239.5 228.5 2.7 239.4 228.5 1.8 0.4)
(pipe-connector 239.4 228.5 1.8 0.4)
(pipe 239.4 228.5 1.8 236.5 228.5 0.9 0.4)
(pipe-connector 236.5 228.5 0.9 0.4)
(pipe 236.5 228.5 0.9 234.5 228.5 0.9 0.4)

(pipe 233.5 231.6 0.9 233.5 234.4 0.9 0.4)

(add-goal 210 222 #f "Shop1")
(add-goal 211 222 #f "Shop1")

(diamond 236.5 233.0)

(add-flag 236 232 50 0 1.5)
(add-flag 226 236 80 0 0.5)
(add-flag 214 218 100 0 0.5)

(define bonus-goto "Shop1")
(set-primary-color (add-goal 219 236 #t "bonusb") 1.0 1.0 1.0)

(cond
 ((= (difficulty) *easy*)
         (set-primary-color (set-acceleration (set-horizon (new-mr-black 235.5 227.5) 3.0) 1.6) 0.5 0.5 0.5)
 )
 ((= (difficulty) *normal*)
         (set-primary-color (set-acceleration (set-modtime
(new-mr-black 235.5 227.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
 ((= (difficulty) *hard*)
         (set-primary-color (set-acceleration (set-modtime
(new-mr-black 235.5 227.5) *mod-spike* -1.) 2.3) 0.8 0.0 0.0)
 )
)

(cond
 ((= (difficulty) *easy*)
         (start-time 400)
 )
 ((= (difficulty) *normal*)
         (start-time 300)
 )
 ((= (difficulty) *hard*)
         (start-time 250)
 )
)




