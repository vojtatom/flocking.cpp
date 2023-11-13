import { vec2 } from "gl-matrix";
import p5 from "p5";
import { DNA } from "./dna";
import { Food } from "./food";
import { VecMath } from "./math";
import { Boundry } from "./quadtree";

export class Agent {
    dna: DNA;
    forces = {
        align : vec2.create(),
        cohes : vec2.create(),
        separ : vec2.create(),
        food  : vec2.create(),
        tmp   : vec2.create(),
        tmp2  : vec2.create(),
        origin: vec2.fromValues(0, 0),
        view  : 0,
        mood  : 0,
        moodNormal: 0,
    };

    count = {
        align: 0,
        cohes: 0,
        separ: 0,
        view:  0,
        happy: 0,
        uncom: 0,
        mood : 0,
    };

    pos!: vec2;
    vel!: vec2;
    acc!: vec2;
    mood = 0;
    moodSwing = 0;
    health = 100;
    fadeAnimation = 0;


    constructor(readonly p: p5){
        this.dna = new DNA(p);
        this.init();
    }

    init() {
        this.pos = vec2.fromValues(this.p.random(this.p.width), this.p.random(this.p.height));
        this.vel = vec2.random(vec2.create(), this.dna.speed);
        this.acc = vec2.random(vec2.create(), this.dna.force);
        this.mood = 0;
        this.moodSwing = 0;
        this.health = 100;
        this.fadeAnimation = 0;
    }
    
    get x() {
        return this.pos[0];
    }

    get y() {
        return this.pos[1];
    }

    edges() {
        if (this.pos[0] > this.p.width) {
            this.pos[0] = 0;
        } else if (this.pos[0] < 0) {
            this.pos[0] = this.p.width;
        }

        if (this.pos[1] > this.p.height) {
            this.pos[1] = 0;
        } else if (this.pos[1] < 0) {
            this.pos[1] = this.p.height;
        }
    }

    flockBoundry() {
        let max = this.dna.maxZone;
        let x = this.pos[0] - max;
        let y = this.pos[1] - max;
        let rad = max * 2;
        return new Boundry(x, y, rad, rad);
    }

    foodBoundry() {
        let max = this.dna.foodZone;
        let x = this.pos[0] - max;
        let y = this.pos[1] - max;
        let rad = max * 2;
        return new Boundry(x, y, rad, rad);
    }

    prepare(){
        //forces
        vec2.set(this.forces.align, 0, 0);
        vec2.set(this.forces.cohes, 0, 0);
        vec2.set(this.forces.separ, 0, 0);
        vec2.set(this.forces.food, 0, 0);
        this.forces.view = 0;
        this.forces.mood = 0;

        //counts
        this.count.align = 0;
        this.count.cohes = 0;
        this.count.separ = 0;
        this.count.view = 0;
        this.count.happy = 0;
        this.count.uncom = 0;
        this.count.mood = 0;
    }

    flock(boids: Agent[]) {
        let dist, angle, dot;

        //empty all temporary vectors
        this.prepare();

        for (let other of boids) {
            dist = vec2.dist(this.pos, other.pos);

            if (other == this) {
                continue;
            }
            
            //alignment
            if (dist < this.dna.alignmentZone) {
                vec2.add(this.forces.align, this.forces.align, other.vel);
                this.count.align++;
            }

            //cohesion
            if (dist < this.dna.cohesionZone) {
                vec2.add(this.forces.cohes, this.forces.cohes, other.pos);
                this.count.cohes++;
            }

            //separation
            if (dist < this.dna.seperationZone) {
                vec2.sub(this.forces.tmp, this.pos, other.pos);
                vec2.scale(this.forces.tmp, this.forces.tmp, 1 / dist);
                vec2.add(this.forces.separ, this.forces.separ, this.forces.tmp);
                this.count.separ++;
            }

            //view
            if (dist < this.dna.viewZone) {
                vec2.sub(this.forces.tmp, other.pos, this.pos);
                angle = vec2.angle(this.vel, this.forces.tmp);
                if (angle < 1){
                    vec2.rotate(this.forces.tmp2, this.vel, this.forces.origin, this.p.HALF_PI);
                    dot = vec2.dot(this.forces.tmp2, this.forces.tmp);
                    this.count.view++;
                    this.forces.view += dot > 0 ? -angle : angle;
                }
            }

            if (dist < this.dna.closeZone) {
                this.count.uncom++;
            } else if (dist < this.dna.happyZone) {
                this.count.happy++;
            }

            if (dist < this.dna.sensitiveZone) {
                this.forces.mood += other.mood;
                this.count.mood++;
            }
        }

        //alignment
        if (this.count.align > 0) {
            vec2.scale(this.forces.align, this.forces.align, 1 / this.count.align);
            VecMath.setMag(this.forces.align, this.dna.speed);            
            vec2.sub(this.forces.align, this.forces.align, this.vel);
            VecMath.limit(this.forces.align, this.dna.force);
            vec2.add(this.acc, this.acc, this.forces.align);
        }

        //cohesion
        if (this.count.cohes > 0) {
            vec2.scale(this.forces.cohes, this.forces.cohes, 1 / this.count.cohes);
            vec2.sub(this.forces.cohes, this.forces.cohes, this.pos);
            VecMath.setMag(this.forces.cohes, this.dna.speed);
            vec2.sub(this.forces.cohes, this.forces.cohes, this.vel);
            VecMath.limit(this.forces.cohes, this.dna.force);
            vec2.add(this.acc, this.acc, this.forces.cohes);
        }

        //separation
        if (this.count.separ > 0) {
            vec2.scale(this.forces.separ, this.forces.separ, 1 / this.count.separ);
            VecMath.setMag(this.forces.separ, this.dna.speed);
            vec2.sub(this.forces.separ, this.forces.separ, this.vel);
            this.forces.moodNormal = (this.dna.moodLimit - this.mood) / (this.dna.moodLimit * 4) + 0.75;
            VecMath.limit(this.forces.separ, this.forces.moodNormal * this.dna.force);
            vec2.add(this.acc, this.acc, this.forces.separ);
        }
        
        //view
        if (this.count.view > 0){
            vec2.rotate(this.forces.tmp, this.vel, this.forces.origin, this.forces.view / this.count.view);
            VecMath.setMag(this.forces.tmp, this.dna.speed);
            vec2.add(this.forces.tmp, this.forces.tmp, this.vel);
            VecMath.limit(this.forces.tmp, this.dna.force);
            vec2.add(this.acc, this.acc, this.forces.tmp);
        }

        //mood
        if (this.count.mood > 0) {
            this.moodSwing += this.dna.moodiness * this.forces.mood / this.count.mood;
        }

        this.moodSwing += this.dna.moodiness * (this.dna.mentalEndurance * (- this.count.uncom * 2 + this.count.happy) + this.dna.mentalHealth);
    }

    eat(food: Food[]) {
        let dist, minDist = -1, target, val;

        //empty all temporary vectors
        this.prepare();

        let f;
        const len = food.length;
        for (let i = 0; i < len; ++i) {
            f = food[i];
            if (f.eaten) {
                continue;
            }
            
            dist = vec2.dist(this.pos, f.pos);
            if ((dist < minDist || minDist == -1) && dist < this.dna.foodZone) {
                vec2.copy(this.forces.food, f.pos)
                minDist = dist;
                target = f;
            }
        }

        if (minDist != -1 && target) {
            if (minDist < 4) {
                val = target.eat();
                this.health += val;
            } else {
                vec2.sub(this.forces.tmp, this.forces.food, this.pos);
                vec2.scale(this.forces.food, this.forces.tmp, 1 / minDist);
                VecMath.setMag(this.forces.food, this.dna.speed);
                vec2.sub(this.forces.food, this.forces.food, this.vel);
                VecMath.limit(this.forces.food, 0.1 * this.dna.force);
                vec2.add(this.acc, this.acc, this.forces.food);
            }
        }

    }

    show() {
        let r, g, b;
        let mood = (this.mood + this.dna.moodLimit) / (this.dna.moodLimit * 2);
        let mentalHealth = (this.dna.mentalHealth + 3) / 6;
        //let moodSwing = (Math.tanh(this.moodSwing) + 1) / 2;
        let health = (1 / -(0.2 * this.health + 1) + 1);

       
        /*b = Math.min(2 - 2 * mentalHealth, 1);
        r = Math.min(2 * mentalHealth, 1);
        g = Math.min(r, b); 
        this.p.stroke(255, 255, 255, 50);
        this.p.ellipse(this.pos[0], this.pos[1], 10, 10); */

        b = Math.min(2 - 2 * mentalHealth, 1);
        r = Math.min(2 * mentalHealth, 1);
        g = Math.min(r, b);
        //this.p.stroke(255 * r, 255 * r / 2, 255 * b, this.fadeAnimation * 50 * health);
        //this.p.ellipse(this.pos[0], this.pos[1], 20, 20);

        b = Math.min(2 - 2 * mood, 1);
        r = Math.min(2 * mood, 1);
        g = Math.min(r, b);
        this.p.fill(255 * r, 130 * r / 2 + 125, 255 * b, this.fadeAnimation * 255 * health);
        //this.p.rect(this.pos[0], this.pos[1], 0.25, 0.25);

        const size = 15;//this.health * 0.2;
        let velnorm = vec2.normalize(this.forces.tmp, this.vel);
        let x = (this.pos[0] - velnorm[0] * 0.5 * size);
        let y = (this.pos[1] - velnorm[1] * 0.5 * size);
        this.p.triangle(this.pos[0], this.pos[1], x - velnorm[1] * 0.2 * size, y + velnorm[0] * 0.2 * size, x + velnorm[1] * 0.2 * size, y - velnorm[0] * 0.2 * size);
        //this.p.ellipse(this.pos[0], this.pos[1], 20, 20);

    }

    update() {
        this.mood = Math.max(Math.min(this.mood + this.moodSwing, this.dna.moodLimit), -this.dna.moodLimit);
        vec2.add(this.pos, this.pos, this.vel);
        vec2.add(this.vel, this.vel, this.acc);
        VecMath.limit(this.vel, this.dna.speed);
        this.health = Math.max(this.health - 0.1 + this.mood / 100, 0);

        vec2.set(this.acc,   0, 0);
        this.moodSwing = 0;
        this.fadeAnimation = Math.min(1, this.fadeAnimation + 0.05)

        if (this.health == 0) {
            this.init();
        }
    }
}