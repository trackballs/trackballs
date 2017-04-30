;;; level child6
;;; This is an easy and short level for young children.
;;; This level was created for my little goddaughter.


(day)

(set-track-name "Pyramid")
(set-author "Attila Boros")

;; Start
(set-start-position 194.5 194.5)
(cond
 ((= (difficulty) *easy*)   (start-time 600))
 ((= (difficulty) *normal*) (start-time 300))
 ((= (difficulty) *hard*)   (start-time 120))
)

;; Goal
(add-goal 200 200 #t "")

;; Do not jump
(jump 0.1)


(trigger 194.5 194.5 0.5 (lambda() (camera-angle 0.0 0.0)))
(trigger 195.5 195.5 0.5 (lambda() (camera-angle 0.0 0.0)))
(trigger 196.5 196.5 0.5 (lambda() (camera-angle 0.0 0.0)))
(trigger 197.5 197.5 0.5 (lambda() (camera-angle 0.0 0.0)))
(trigger 198.5 198.5 0.5 (lambda() (camera-angle 0.0 0.3)))

(trigger 194.5 206.5 0.5 (lambda() (camera-angle 1.0 0.0)))
(trigger 195.5 205.5 0.5 (lambda() (camera-angle 1.0 0.0)))
(trigger 196.5 204.5 0.5 (lambda() (camera-angle 1.0 0.0)))
(trigger 197.5 203.5 0.5 (lambda() (camera-angle 1.0 0.0)))
(trigger 198.5 202.5 0.5 (lambda() (camera-angle 1.0 0.3)))

(trigger 206.5 194.5 0.5 (lambda() (camera-angle -1.0 0.0)))
(trigger 205.5 195.5 0.5 (lambda() (camera-angle -1.0 0.0)))
(trigger 204.5 196.5 0.5 (lambda() (camera-angle -1.0 0.0)))
(trigger 203.5 197.5 0.5 (lambda() (camera-angle -1.0 0.0)))
(trigger 202.5 198.5 0.5 (lambda() (camera-angle -1.0 0.3)))

(trigger 206.5 206.5 0.5 (lambda() (camera-angle 2.0 0.0)))
(trigger 205.5 205.5 0.5 (lambda() (camera-angle 2.0 0.0)))
(trigger 204.5 204.5 0.5 (lambda() (camera-angle 2.0 0.0)))
(trigger 203.5 203.5 0.5 (lambda() (camera-angle 2.0 0.0)))
(trigger 202.5 202.5 0.5 (lambda() (camera-angle 2.0 0.3)))

(add-cyclic-platform 206 200 206 200 -7.0 -6.0 1.0 0.5)
(add-cyclic-platform 204 201 204 201 -5.0 -4.0 0.3 0.4)
(add-cyclic-platform 198 200 198 200 -3.0 -2.0 0.5 0.6)

(trigger 203.5 194.5 0.5 (lambda() (set-cell-heights 203 194 204 194 -7.5 -7.5 -7.5 -7.5 -7.5)))

(trigger 199.5 196.5 0.5 (lambda() (set-cell-heights 199 196 200 196 -6.0 -5.0 -6.0 -5.0 -5.0)))

(set-cell-wall-colors 185 194 190 203 0 .9 .9 .6)
(set-cell-wall-colors 185 194 190 203 1 .9 .9 .6)
(set-cell-wall-colors 185 194 190 203 2 .9 .9 .6)
(set-cell-wall-colors 185 194 190 203 3 .9 .9 .6)

(set-cell-wall-colors 192 192 208 208 0 .9 .7 .3)
(set-cell-wall-colors 192 192 208 208 1 .9 .7 .3)
(set-cell-wall-colors 192 192 208 208 2 .9 .7 .3)
(set-cell-wall-colors 192 192 208 208 3 .9 .7 .3)

