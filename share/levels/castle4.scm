;;; level castle4
;;; This level was designed for experienced trackballs players.
;;; Created by Attila Boros. 2006

;; If you couldn't go through the maze, you can find some help in the end of this file.


(day)
(snow 1.0)
(fog 0.0)
(map-is-transparent #t)

(set-track-name "Ice and Glass")
(set-author "Attila Boros")

;; Start
(set-start-position 5.5 5.5)
(cond
 ((= (difficulty) *easy*)   
    (start-time 480) 
    (add-modpill 27.5 26.5 *mod-extra-life* 10 0)
    (diamond  6.0 30.0)
    (diamond 30.5 23.5)
    (diamond 41.0 39.0)
    (diamond 43.0 32.0)
 )
 ((= (difficulty) *normal*) 
    (start-time 360) 
    (add-modpill 27.5 26.5 *mod-extra-life* 10 0)
    (diamond 30.5 23.5)
    (diamond 41.0 39.0)
    (diamond 43.0 32.0)
 )
 ((= (difficulty) *hard*)   
    (start-time 300)
 )
)



;; Goal
(add-goal 48 12 #f "")

;; Jump
(jump 1.0)


(set-cell-wall-colors 0 0 255 255 0 .1 .3 .9 .5)
(set-cell-wall-colors 0 0 255 255 1 .1 .3 .9 .5)
(set-cell-wall-colors 0 0 255 255 2 .1 .3 .9 .5)
(set-cell-wall-colors 0 0 255 255 3 .1 .3 .9 .5)


;; The Gate
;; Close the gate
(trigger 14 16 1.5 (lambda() (set-cell-heights 10 10 10 10 -2.5 -2.5 -2.5 -2.5 -2.5)))
;; Open the gate
(trigger 9.5 9.5 1.0 (lambda() (set-cell-heights 10 10 10 10 -8.0 -8.0 -8.0 -8.0 -8.0)))


;; Maze of PI
(trigger  8.5 32.5 3.0  (lambda() (fog 1.0)))
(trigger 38.5 35.5 1.0 (lambda() (fog 0.0)))
(trigger  5.5  5.5 1.0 (lambda() (fog 0.0)))
(sign "Maze of PI" 1.0 50.0 -10.0 9.5 33.5)
(sign "Secret stairs somewhere" 1.0 50.0 -10.0 10.5 39.5)

;; Stairs is appear
(trigger 12 74 1 (lambda() 
(set-cell-heights 11 73 11 74 -7.5 -7.5 -7.5 -7.5 -7.5)
(set-cell-heights 10 73 10 74 -7.0 -7.0 -7.0 -7.0 -7.0)
(set-cell-heights  9 73  9 74 -6.5 -6.5 -6.5 -6.5 -6.5)
(set-cell-heights  8 73  8 74 -6.0 -6.0 -6.0 -6.0 -6.0)
(set-cell-heights  7 73  7 74 -5.5 -5.5 -5.5 -5.5 -5.5)
(set-cell-heights  6 73  6 74 -5.0 -5.0 -5.0 -5.0 -5.0)
(set-cell-heights  5 73  5 74 -4.5 -4.5 -4.5 -4.5 -4.5)
(set-cell-heights  5 75  5 75 -4.5 -4.5 -4.5 -4.5 -4.5)
(set-cell-heights  5 76  5 76 -4.0 -4.0 -4.0 -4.0 -4.0)
(set-cell-heights  5 77  5 77 -3.5 -3.5 -3.5 -3.5 -3.5)
(set-cell-heights  5 78  5 78 -3.0 -3.0 -3.0 -3.0 -3.0)
(set-cell-heights  5 79  5 79 -2.5 -2.5 -2.5 -2.5 -2.5)
(set-cell-heights  5 72  5 72 -4.5 -4.5 -4.5 -4.5 -4.5)
(set-cell-heights  5 71  5 71 -4.0 -4.0 -4.0 -4.0 -4.0)
(set-cell-heights  5 70  5 70 -3.5 -3.5 -3.5 -3.5 -3.5)
(set-cell-heights  5 69  5 69 -3.0 -3.0 -3.0 -3.0 -3.0)
(set-cell-heights  5 68  5 68 -2.5 -2.5 -2.5 -2.5 -2.5)
))

;; Stairs is disappear
(trigger 8.5 32.5 3.0 (lambda() 
(set-cell-heights 11 73 11 74 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights 10 73 10 74 -8.0 -8.0 -8.0 -8.0 -8.0)
(set-cell-heights  9 73  9 74 -8.0 -8.0 -8.0 -8.0 -8.0)
(set-cell-heights  8 73  8 74 -8.0 -8.0 -8.0 -8.0 -8.0)
(set-cell-heights  7 73  7 74 -8.0 -8.0 -8.0 -8.0 -8.0)
(set-cell-heights  6 73  6 74 -8.0 -8.0 -8.0 -8.0 -8.0)
(set-cell-heights  5 73  5 74 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 75  5 75 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 76  5 76 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 77  5 77 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 78  5 78 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 79  5 79 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 72  5 72 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 71  5 71 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 70  5 70 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 69  5 69 -2.0 -2.0 -2.0 -2.0 -2.0)
(set-cell-heights  5 68  5 68 -2.0 -2.0 -2.0 -2.0 -2.0)
))

(trigger 11.5 51.5 1.5 (lambda() (diamond 11.5 51.5)))
(trigger 16.5 41.5 1.5 (lambda() (diamond 16.5 41.5)))


;; If you are lost in maze of PI, try this:
;; Get the digits of PI after the decimal dot.
;; From the entrance of maze turn left by the next turn-out if the digit is uneven,
;; and turn right if the digit is even.
;; So you reach a dead-end. Go close to the wall in front of you and the secret stairs appear.
