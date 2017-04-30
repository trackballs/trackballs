;;; level castle3
;;; This level was designed for experienced trackballs players.
;;; Created by Attila Boros. 2006


(day)

(set-track-name "Stronghold")
(set-author "Attila Boros")

;; Start
(set-start-position 245.5 247.5)

(cond
 ((= (difficulty) *easy*)   
    (start-time 600) 
    (diamond 240.5 237.5)(diamond 254.5 234.5)(diamond 253.5 252.5)(diamond 237.5 247.5)
 )
 ((= (difficulty) *normal*) 
    (start-time 480) 
    (diamond 240.5 237.5)(diamond 254.5 234.5)
 )
 ((= (difficulty) *hard*)   
    (start-time 300)
 )
)

;; Goal
(add-goal 232 216 #f "castle4")

;; Do not jump
(jump 0.1)

(add-flag 245 244 50 1 0.1) 
(add-flag 253 253 50 1 0.1) 
(add-flag 246 216 50 1 0.1) 
(add-flag 245 217 50 1 0.1) 
(add-flag 247 225 50 1 0.1) 
(add-flag 252 225 10 1 0.1) 
(add-flag 252 217 10 1 0.1) 
(add-flag 235 221 10 1 0.1) 
(add-flag 244 221 10 1 0.1) 
(add-flag 247 242 10 1 0.1) 
(add-flag 242 241 10 1 0.1) 
(add-flag 242 238 10 1 0.1) 
(add-flag 239 241 10 1 0.1) 
(add-flag 239 238 10 1 0.1) 
(add-flag 235 237 10 1 0.1) 
(add-flag 249 222 10 1 0.1) 
(add-flag 231 226 200 1 0.1) 


(define pipe1 (pipe 252.5 227.4 -2.5 252.5 226.0 -2.5 0.5))
(pipe-connector 252.5 226.0 -2.5 0.6)
(define pipe2 (pipe 252.5 226.0 -2.5 252.5 225.0 -0.5 0.5))
(set-wind pipe2 10.0 0)
(pipe-connector 252.5 225.0 -0.5 0.6)
(define pipe3 (pipe 252.5 225.0 -0.5 252.5 223.6 -0.5 0.5))

(define pipe4 (pipe 252.5 219.4 -0.5 252.5 218.5 -0.5 0.5))
(pipe-connector 252.5 218.5 -0.5 0.6)
(define pipe5 (pipe 252.5 218.5 -0.5 251.5 217.5 -2.5 0.5))
(set-wind pipe5 0.0 -10.0)
(pipe-connector 251.5 217.5 -2.5 0.6)
(define pipe6 (pipe 251.5 217.5 -2.5 250.5 217.5 -2.5 0.5))

(set-cell-wall-colors 0 0 255 255 0 .6 .3 .1)
(set-cell-wall-colors 0 0 255 255 1 .6 .3 .1)
(set-cell-wall-colors 0 0 255 255 2 .6 .3 .1)
(set-cell-wall-colors 0 0 255 255 3 .6 .3 .1)

;;Secret teleports
(trigger 253.5 220.5 0.5 (lambda() (add-teleport 251 220 250 230 0.25)(add-teleport 251 230 252 220 0.25)))

;;Bonus Life
(add-modpill 246.5 229.5 *mod-extra-life* 10 0)

