(set-track-name "Ultimate")
(set-author "Ulrik Enstad")
(set-start-position 250.5 250.5)

(pipe 239.0 236.0 4.95 234.0 236.0 4.95 0.08)
(pipe 239.0 235.8 4.95 234.0 235.8 4.95 0.08)
(pipe 239.0 235.6 4.95 234.0 235.6 4.95 0.08)
(pipe 239.0 235.4 4.95 234.0 235.4 4.95 0.08)
(pipe 239.0 235.2 4.95 234.0 235.2 4.95 0.08)
(pipe 239.0 235.0 4.95 234.0 235.0 4.95 0.08)
(pipe 239.0 234.8 4.95 234.0 234.8 4.95 0.08)
(pipe 239.0 234.6 4.95 234.0 234.6 4.95 0.08)
(pipe 239.0 234.4 4.95 234.0 234.4 4.95 0.08)
(pipe 239.0 234.2 4.95 234.0 234.2 4.95 0.08)
(pipe 239.0 234.0 4.95 234.0 234.0 4.95 0.08)
(pipe 239.0 234.0 4.95 234.0 234.0 4.95 0.08)
(pipe 236.0 238.0 4.95 234.0 238.0 4.95 0.08)
(pipe 236.0 237.8 4.95 234.0 237.8 4.95 0.08)
(pipe 236.0 237.6 4.95 234.0 237.6 4.95 0.08)
(pipe 236.0 237.4 4.95 234.0 237.4 4.95 0.08)
(pipe 236.0 237.2 4.95 234.0 237.2 4.95 0.08)
(pipe 236.0 237.0 4.95 234.0 237.0 4.95 0.08)
(pipe 236.0 236.8 4.95 234.0 236.8 4.95 0.08)
(pipe 236.0 236.6 4.95 234.0 236.6 4.95 0.08)
(pipe 236.0 236.4 4.95 234.0 236.4 4.95 0.08)
(pipe 236.0 236.2 4.95 234.0 236.2 4.95 0.08)


(add-teleport 244 250 244 244 0.1)
(add-teleport 250 243 244 244 0.1)

(set-cell-velocity 228 239 235 242 -3.0 3.0)

(cond
 ((= (difficulty) *easy*)
         (set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 231 233) 0.0 0.3 1.0) 0.0 0.5 0.0) 6.5) 3.0)
 )
 ((= (difficulty) *normal*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color
(new-mr-black 231 233)
1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
 ((= (difficulty) *hard*)
         (set-acceleration (set-modtime (set-primary-color (set-secondary-color
(new-mr-black 231 233)
1.0 1.0 1.0) 0.0 0.3 1.0) *mod-speed* -1.) 8.0)
 )
)


(diamond 208.0 236.0)

(set-modtime (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 225.0 211.0) 1.0 1.0 1.0) 0.5 1.0 0.0) 8.0) *mod-large* -1.)

(set-modtime (set-primary-color (set-acceleration (new-mr-black 218.5 211.5) 2.3) 0.8 0.0 0.0) *mod-spike* -1.)

(add-goal 225 216 #f "frg10")
(add-goal 224 216 #f "frg10")

(define bonus-goto "frg10")
(set-primary-color (add-goal 199 227 #t "bonusb") 1.0 1.0 1.0)

(add-flag 210 236 30 0 0.5)
(add-flag 218 239 30 0 1.5)
(add-flag 208 219 30 0 1.5)
(add-flag 209 216 30 0 1.5)

(cond
 ((= (difficulty) *easy*)
         (define speed .6)
 )
 ((= (difficulty) *normal*)
         (define speed .4)
 )
 ((= (difficulty) *hard*)
         (define speed .2)
 )
)

(add-cyclic-platform 220 238 220 238 3.0 5.0 1. speed)
(add-cyclic-platform 221 238 221 238 3.0 5.0 3. speed)
(add-cyclic-platform 220 239 220 239 3.0 5.0 4. speed)
(add-cyclic-platform 221 239 221 239 3.0 5.0 2. speed)

(set-horizon (set-acceleration (set-primary-color (set-secondary-color (new-mr-black 207.5 233.5) 0.0 0.3 1.0) 0.0 0.5 0.0)6.5) 3.0)

(jump 0.0)

(cond
 ((= (difficulty) *easy*)
         (start-time 240)
 )
 ((= (difficulty) *normal*)
         (start-time 120)
 )
 ((= (difficulty) *hard*)
         (start-time 100)
 )
)

(restart-time 0)

