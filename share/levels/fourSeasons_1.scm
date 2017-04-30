;;; The four seasons - Winter (level 1)

(day)
(snow 0.3)
(fog 0.1)
(fog-color 0.7 0.7 0.9)
(set-track-name "Winter")
(set-author "Mathias Broxvall")

(start-time 180) 
(set-start-position 202.5 199.5)

(cond ((= (difficulty) *easy*) (add-modpill 196.5 201.5 *mod-large* 30 -1)))
(cond ((= (difficulty) *easy*) (add-modpill 198.5 169.5 *mod-spike* 60 30)))

(define enemy1 (new-mr-black 198.5 173.5))
(cond
 ((= (difficulty) *easy*) (set-acceleration enemy1 3.0))
 ((= (difficulty) *normal*) (set-acceleration enemy1 4.0))
 ((= (difficulty) *hard*) (set-acceleration enemy1 5.0)))

(diamond 183.5 164.5)
(add-modpill 180.5 167.5 *mod-small* 30 -1)

(add-flag 191 174 100 1 0.1)
(add-flag 175 167 100 1 0.1)

;; Small pipe to the bonus area
(define pipe0 (pipe 176.5 167.5 -0.7 176.5 175.5 -0.7 0.25))
(set-wind pipe0 5.0 -5.0)
(set-primary-color pipe0 1.0 1.0 0.2 0.6)

(add-modpill 177.5 177.5 *mod-extra-life* 30 -1)
(add-flag 178 176 100 1 0.1) 
(add-flag 175 178 100 1 0.1)
(add-flag 161 153 100 1 0.1)

;; more pipes
(define pipe1 (pipe 176.5 164.5 -0.6 176.5 154.5 -0.6 0.4))
(set-wind pipe1 3.0 -3.0)
(define pipe2 (pipe 175.5 153.5 -0.6 171.0 153.5 -0.6 0.4))
(set-wind pipe2 0.0 -0.0)
(define pipe3 (pipe 170.0 153.5 -0.6 165.0 153.5 -1.6 0.4))
(set-wind pipe3 3.0 -3.0)
(define pipe4 (pipe 170.5 153.0 -0.6 170.5 143.5  0.4 0.4))
(set-wind pipe4 3.0 -3.0)
(define pipe5 (pipe 179.5 141.0  0.4 179.5 152.0  1.4 0.4))
(set-wind pipe5 3.0 -3.0)

(if (not (= (difficulty) *easy*)) (add-spike 170.5 153.5 0.30 -0.00))
(if (= (difficulty) *hard*) (add-spike 175.5 140.5 0.30 -0.00))


;; Goal!
(add-goal 179 154 #f "fourSeasons_2")

