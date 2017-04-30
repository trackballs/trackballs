;; The source season - Spring (level 2)

(day)
(rain 0.5)
(fog 0.01)
(fog-color 0.95 0.95 1.0)
(set-track-name "Spring")
(set-author "Mathias Broxvall")
(map-is-transparent #t)

(start-time 180) 
(set-start-position 199.5 199.5)
(add-goal 193 203 #f "fourSeasons_3")

(define wind-pos 2.0)
(define wind-neg 2.0)

(define pipe0 (pipe 199.5 193.0 -1.6 199.5 177.0 -0.5 0.39))
(set-wind pipe0 wind-pos wind-neg)
(set-primary-color pipe0 0.5 0.5 1.0 1.0)
(define fountain-0 (fountain 199.5 177.9 -0.7 0.04 0.2 400.0))
(set-fountain-velocity fountain-0 0.0 -0.2 0.15)


(if (> (difficulty) *easy*)
	(begin
	  (define enemy-1 (new-mr-black 198.5 175.5))
	  (if (= (difficulty) *hard*) (set-modtime enemy-1 *mod-spike* -1.))
	  (set-acceleration enemy-1 2.0)
	  ))

(define spike-speed 0.2)
(cond
 ((= (difficulty) *easy*) (set! spike-speed 0.07))
 ((= (difficulty) *normal*) (set! spike-speed 0.12))
 ((= (difficulty) *hard*) (set! spike-speed 0.15))
)
(add-spike 198.5 173.5 spike-speed -0.00)
(add-spike 199.5 173.5 spike-speed -0.50)
(add-spike 200.5 173.5 spike-speed -1.00)

(define ff-1 (forcefield 205.5 176.5 0.0 0.0 -2.0 0.0 0.5 *ff-kill*))
(set-primary-color ff-1 0.2 0.2 1.0)
(switch 201.5 173.5 (lambda()(set-onoff ff-1 #t)) (lambda()(set-onoff ff-1 #f))) 

(define pipe1 (pipe 206.0 175.5 -1.6 221.0 175.5 -0.5 0.39))
(set-wind pipe1 wind-pos wind-neg)
(set-primary-color pipe1 0.5 0.5 1.0 1.0)
(define fountain-1 (fountain 220.1 175.5 -0.7 0.04 0.2 400.0))
(set-fountain-velocity fountain-1 0.2 0.0 0.15)

(define pipe2 (pipe 223.5 181.5 -1.6 223.5 197.0 -0.5 0.39))
(set-wind pipe2 wind-pos wind-neg)
(set-primary-color pipe2 0.5 0.5 1.0 1.0)
(define fountain-2 (fountain 223.5 196.1 -0.7 0.04 0.2 400.0))
(set-fountain-velocity fountain-2 0.0 0.2 0.15)

(define enemy-2 (new-mr-black 223.5 174.5))
(if (> (difficulty) *easy*) (set-modtime enemy-2 *mod-spike* -1.))
(set-acceleration enemy-2 2.0)

(define pipe3 (pipe 217.0 199.5 -1.6 201.5 199.5 -0.5 0.39))
(set-wind pipe3 wind-pos wind-neg)
(set-primary-color pipe3 0.5 0.5 1.0 1.0)
(define fountain-3 (fountain 201.9 199.5 -0.7 0.04 0.2 400.0))
(set-fountain-velocity fountain-3 -0.2 0.0 0.15)

(define enemy-3 (new-mr-black 224.5 199.5))
(if (= (difficulty) *hard*) (set-modtime enemy-3 *mod-spike* -1.))
(set-acceleration enemy-3 2.0)


(define ff-0 (forcefield 197.1 200.8 0.0 0.0 -2.6 0.0 0.5 *ff-kill*))
(set-primary-color ff-0 0.2 0.2 1.0)
(switch 225.5 197.5 (lambda()(set-onoff ff-0 #t)) (lambda()(set-onoff ff-0 #f))) 

(add-flag 199 175 100 1 0.1) 
(add-flag 223 175 100 1 0.1) 
(add-flag 223 199 100 1 0.1) 
