;;; lv7 - slalom

(set-track-name "Slalom") (day)
(set-author "Mathias Broxvall")
(start-time (- 120 (* (difficulty) 30)))
(set-start-position 238.5 232.5)
(fog 0.5)
(fog-color 0.9 0.9 1.0)

(define (red-flag x y p)
  (set-primary-color (add-flag x y 0 1 -1.0) 0.8 0.2 0.2)
  (add-flag x (+ y 2) p 0 (- 2.0 (* 0.5 (difficulty)))))
(define (blue-flag x y p)
  (set-primary-color (add-flag x y 0 1 -1.0) 0.2 0.2 0.8)
  (add-flag x (- y 2) p 0 (- 2.0 (* 0.5 (difficulty)))))

(blue-flag 234 230 100)
(red-flag 231 228 100)
(blue-flag 229 227 100)
(red-flag 224 229 100)

(define mb0 (new-mr-black 221.5 229.5))
(set-texture mb0 "track.png")
(set-primary-color mb0 0.8 0.5 0.5)
(set-acceleration mb0 (+ 2.0 (* 1.0 (difficulty))))
(if (> (difficulty) *easy*) (set-modtime mb0 *mod-spike* -1.))
(set-horizon mb0 (+ 5 (* 3 (difficulty))))

(blue-flag 218 227 150)
(red-flag 213 229 150)
(set-primary-color (add-flag 203 228 500 1 0.1) 0.9 0.8 0.2) ; A golden flag, 500 points
(red-flag 197 230 150)
(blue-flag 193 224 150)
(set-primary-color (add-flag 186 230 500 1 0.1) 0.9 0.8 0.2) ; A golden flag, 500 points

(define mb1 (new-mr-black 186.5 222.5))
(set-texture mb1 "track.png")
(set-primary-color mb1 0.5 0.5 0.9)
(set-acceleration mb1 (+ 2.0 (* 1.0 (difficulty))))
(if (= (difficulty) *hard*) (set-modtime mb1 *mod-spike* -1.))
(set-horizon mb1 (+ 5 (* 3 (difficulty))))

(set-primary-color (add-flag 179 216 500 1 0.1) 0.9 0.8 0.2) ; A golden flag, 500 points
(add-spike 179.5 216.5 0.2 -0.00)
(red-flag 172 212 100)
(blue-flag 169 206 100)
(red-flag 164 207 100)
(add-goal 158 205 #t "lv8")
