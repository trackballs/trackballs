;;; level child4
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)
(fog 0.1)

(set-track-name "Big Jump")
(set-author "Attila Boros")

;; Start
(set-start-position 242.5 252.5)
(start-time 300) 
(cond
 ((= (difficulty) *easy*)   (start-time 300))
 ((= (difficulty) *normal*) (start-time 180))
 ((= (difficulty) *hard*)   (start-time 120))
)

;; Goal
(add-goal 242 226 #f "child5")

;; Do not jump
(jump 0.1)


(add-cyclic-platform 245 247 245 247 -3.0 0.0 1.0 0.5)

(add-cyclic-platform 249 241 249 241 -2.0 2.0 1.0 0.8)

(define pipe1(pipe 246.9  252.5  0.5 250.1 252.5 0.5 0.5))
(set-primary-color pipe1 .3 .7 .9 .5)

(define pipe2(pipe 250.2  247.2  0.5 247.5 244.5 -1.5 0.5))
(set-primary-color pipe2 .3 .7 .9 .5)

(define pipe3(pipe 250.5  235.6  3.0 248.5 230.0 -6.5 0.5))
(set-primary-color pipe3 .3 .7 .9 .5)
(set-wind pipe3 20.0 20.0)
(define pc1(pipe-connector 248.5 230.0 -6.5 0.5))
(define pipe4(pipe 248.5 230.0 -6.5 246.5 228.0 -6.0 0.5))
(set-primary-color pipe4 .3 .7 .9 .5)
(set-wind pipe4 40.0 40.0)
(define pc2(pipe-connector 246.5 228.  -6.0 0.5))
(define pipe5(pipe 246.5 228. -6.0 245.5 228.0 -1.0 0.5))
(set-primary-color pipe5 .3 .7 .9 .5)
(set-wind pipe5 60.0 60.0)
(define pc3(pipe-connector 245.5 228.0 -1.0 0.5))
(set-primary-color pc1 .3 .7 .9 .4)
(set-primary-color pc2 .3 .7 .9 .4)
(set-primary-color pc3 .3 .7 .9 .4)
(define pipe5(pipe 245.5 228.0 -1.0 242.5 228.0 0.0 0.5))
(set-primary-color pipe5 .3 .7 .9 .5)
(set-wind pipe5 2.0 2.0)

(define pipe6(pipe 251.1  228.5 -3.6 244.5 230.5 -3.6 0.5))
(set-primary-color pipe6 .9 .3 .7 .5)

(set-cell-wall-colors 0 0 255 255 0 .9 .6 .0)

(set-cell-water-heights 0 0 255 255 -9 -9 -9 -9 -9)

(diamond 250.5 241.5)

(add-flag 246 238 30 1 .2)
(add-flag 244 238 30 1 .2)
(add-flag 235 231 20 1 .2)
(add-flag 235 227 20 1 .2)
(add-flag 239 227 20 1 .2)

