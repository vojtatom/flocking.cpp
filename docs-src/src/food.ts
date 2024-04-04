import { vec2 } from "gl-matrix";
import p5 from "p5";


export class Food {
    pos: vec2;
    value = 10;
    available = true;

    constructor(readonly p: p5) {
        this.pos = vec2.fromValues(p.random(p.width), p.random(p.height));
    }

    get x() {
        return this.pos[0];
    }

    get y() {
        return this.pos[1];
    }

    get eaten() {
        return !this.available;
    }

    refresh() {
        this.available = true;
    }

    show() {
        this.p.stroke(255, 40);
        this.p.point(this.pos[0], this.pos[1]);
    }

    eat() {
        this.available = false;
        this.pos = vec2.fromValues(this.p.random(this.p.width), this.p.random(this.p.height));
        return this.value;
    }
}