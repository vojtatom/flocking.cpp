import { Agent } from "./agent";
import { Food } from "./food";
import { Quadtree } from "./quadtree";
import p5 from "p5";

class Sketch {
    flock: Agent[] = [];
    food: Food[] = [];
    boidTree!: Quadtree<Agent>;
    foodTree!: Quadtree<Food>;
    canvas!: p5.Renderer;

    constructor(readonly p: p5) {
        const container = document.getElementById("sketch");

        console.log(container);
        let w = container!.clientWidth;
        let h = container!.clientHeight;

        console.log(w, h);
        this.canvas = p.createCanvas(w, h);
        this.canvas.parent("sketch");
        this.boidTree = new Quadtree<Agent>(this.p);
        this.foodTree = new Quadtree<Food>(this.p);
        this.init(w, h);
    }

    init(w: number, h: number) {
        this.flock = [];
        this.food = [];

        this.boidTree.reset();
        this.boidTree.init(0, 0, w, h);
        this.foodTree.reset();
        this.foodTree.init(0, 0, w, h);

        for (let i = 0; i < 200; i++) this.flock.push(new Agent(this.p));

        for (let i = 0; i < 100; i++) this.food.push(new Food(this.p));
    }

    resize() {
        const container = document.getElementById("sketch");
        let w = container!.clientWidth;
        let h = container!.clientHeight;
        this.p.resizeCanvas(w, h);
        this.init(w, h);
    }

    draw() {
        let bound;
        let found;

        this.p.background(40);

        for (let boid of this.flock) {
            boid.update();
            boid.edges();
            this.boidTree.insert(boid);
        }

        this.p.noFill();
        this.p.strokeWeight(4);

        for (let f of this.food) {
            this.foodTree.insert(f);
            f.show();
            f.refresh();
        }

        this.p.strokeWeight(1);
        this.boidTree.show();

        this.p.noStroke();

        for (let boid of this.flock) {
            boid.show();

            bound = boid.foodBoundry();
            found = this.foodTree.query(bound);
            boid.eat(found);

            bound = boid.flockBoundry();
            found = this.boidTree.query(bound);
            boid.flock(found);
        }

        this.boidTree.dispose();
        this.foodTree.dispose();
    }
}

function sketch(p: p5) {
    let s: Sketch;

    p.setup = function () {
        s = new Sketch(p);
        p.windowResized = () => s.resize();
    };

    p.draw = function () {
        s.draw();
    };
}

const myp5 = new p5(sketch);

function isScrolledIntoView(el: HTMLElement) {
    var rect = el.getBoundingClientRect();
    var elemTop = rect.top;
    var elemBottom = rect.bottom;

    // Only completely visible elements return true:
    let isVisible =
        elemTop >= -window.innerHeight * 0.5 &&
        elemBottom <= 1.5 * window.innerHeight;
    // Partially visible elements return true:
    //let isVisible = elemTop < window.innerHeight && elemBottom >= 0;
    return isVisible;
}

let last_state = false;
document.onscroll = (e) => {
    let canvas = document.getElementById("sketch");
    let visible = false;

    if (canvas) visible = isScrolledIntoView(canvas);

    if (last_state != visible && myp5) {
        if (!visible) {
            myp5.noLoop();
        } else {
            myp5.loop();
        }
        last_state = visible;
    }
};
