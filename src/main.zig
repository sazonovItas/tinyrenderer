const std = @import("std");
const x11 = @import("x11.zig");
const pixel = @import("pixel.zig");
const model = @import("model.zig");
const math = @import("math.zig");
const Buffer2D = @import("buffer.zig").Buffer2D;
const SphereCamera = @import("camera.zig").SphereCamera;

const Error = error{
    FailedOpenDisplay,
    FailedFindRootWindow,
    FailedCreateWindow,
    FailedCreateGC,
    FailedInternAtom,
    FailedAllocColor,
};

pub fn createWindow(display: ?*x11.Display, screen: c_int, visual: [*c]x11.Visual) !x11.Window {
    const root_window = x11.DefaultRootWindow(display);
    const colormap = x11.XCreateColormap(display, root_window, visual, x11.AllocNone);

    var set_window_attrs = x11.XSetWindowAttributes{
        .colormap = colormap,
        .event_mask = x11.StructureNotifyMask | x11.KeyPressMask | x11.KeyReleaseMask |
            x11.PointerMotionMask | x11.ButtonPressMask | x11.ButtonReleaseMask |
            x11.FocusChangeMask | x11.VisibilityChangeMask | x11.ExposureMask |
            x11.EnterWindowMask | x11.LeaveWindowMask | x11.PropertyChangeMask |
            x11.Button1MotionMask | x11.Button5Mask | x11.Button4Mask,
    };

    const window = x11.XCreateWindow(
        display,
        root_window,
        0,
        0,
        @intCast(x11.XDisplayWidth(display, screen)),
        @intCast(x11.XDisplayHeight(display, screen)),
        0,
        x11.DefaultDepth(display, screen),
        x11.InputOutput,
        visual,
        x11.CWColormap | x11.CWEventMask,
        &set_window_attrs,
    );

    if (window == 0) {
        return Error.FailedCreateWindow;
    }

    return window;
}

pub const DB = struct {
    display: ?*x11.Display,
    window: x11.Window,
    gc: x11.GC,
    pixels: Buffer2D(pixel.BGRA),
    image: ?*x11.XImage,
    mesh: model.Mesh,
    allocator: std.mem.Allocator,
    camera: SphereCamera,
    vertices: []math.Vec4f,
};

pub fn line(db: DB, p1: math.Vec2f, p2: math.Vec2f, color: pixel.BGRA) void {
    var x0: i32 = @intFromFloat(p1.x);
    var y0: i32 = @intFromFloat(p1.y);
    var x1: i32 = @intFromFloat(p2.x);
    var y1: i32 = @intFromFloat(p2.y);

    var steep: bool = false;
    if (math.abs(x0 - x1) < math.abs(y0 - y1)) {
        std.mem.swap(i32, &x0, &y0);
        std.mem.swap(i32, &x1, &y1);
        steep = true;
    }

    if (x0 > x1) {
        std.mem.swap(i32, &x0, &x1);
        std.mem.swap(i32, &y0, &y1);
    }

    const dx = x1 - x0;
    const dy = y1 - y0;

    const derr2: i32 = math.abs(dy) * 2;
    var err2: i32 = 0;
    var x, var y = .{ x0, y0 };
    while (x <= x1) {
        if (steep) {
            db.pixels.set(y, x, color);
        } else {
            db.pixels.set(x, y, color);
        }

        err2 += derr2;
        if (err2 > dx) {
            y += if (y1 > y0) 1 else -1;
            err2 -= dx * 2;
        }

        x += 1;
    }

    return;
}

pub fn redraw(db: *DB) void {
    var wa: x11.XWindowAttributes = undefined;
    _ = x11.XGetWindowAttributes(db.*.display, db.*.window, &wa);

    db.*.pixels.clear(std.mem.zeroes(pixel.BGRA));

    const aspect: f32 = @as(f32, @floatFromInt(wa.width)) / @as(f32, @floatFromInt(wa.height));
    const proj = math.M44.perspective(math.radians(90.0), aspect, 0.1, 100.0);
    // const proj = math.M44.orto(aspect, 1.0, 1.0, 100.0);
    const viewport = math.M44.viewport(wa.x, wa.y, wa.width, wa.height, 2.0);

    const mat = proj.mul(db.*.camera.view());

    const modelMat = mat.mul(math.M44.translate_mat(math.Vec3f.from(0.0, -1.0, 0.0)));
    const vertices = db.*.vertices;
    for (0.., db.*.mesh.vertices) |i, v| {
        vertices[i] = modelMat.apply_to_vec4(math.Vec4f.from(v[0], v[1], v[2], 1.0)).perspective_div();
    }

    for (db.*.mesh.objects) |obj| {
        var i: usize = 0;
        for (obj.face_vertices) |cnt| {
            for (i..(i + cnt - 1)) |idx| {
                var p1, var p2 = .{ vertices[obj.indices[idx].vertex], vertices[obj.indices[idx + 1].vertex] };

                if (p1.w < 0.0 or p2.w < 0.0) {
                    continue;
                }

                p1, p2 = .{ p1.perspective_div(), p2.perspective_div() };
                if (p1.z < 0.1 or p1.z > 100.0 or p2.z < 0.1 or p2.z > 100.0) {
                    continue;
                }

                const mnP: math.Vec2f = math.Vec2f.from(@min(p1.x, p2.x), @min(p1.y, p2.y));
                const mxP: math.Vec2f = math.Vec2f.from(@max(p1.x, p2.x), @max(p1.y, p2.y));

                if (mxP.x < -1.0 or mxP.y < -1.0 or mnP.x > 1.0 or mnP.y > 1.0) {
                    continue;
                }

                p1, p2 = .{ viewport.apply_to_vec4(p1), viewport.apply_to_vec4(p2) };

                line(db.*, math.Vec2f.from(p1.x, p1.y), math.Vec2f.from(p2.x, p2.y), pixel.BGRA.make(0xff, 0xff, 0xff, 0x00));
            }

            i += cnt;
        }
    }

    const pointX1 = math.Vec4f.from(-1.0, 0.0, 0.0, 1.0);
    const pointX2 = math.Vec4f.from(1.0, 0.0, 0.0, 1.0);
    const pointY1 = math.Vec4f.from(0.0, -1.0, 0.0, 1.0);
    const pointY2 = math.Vec4f.from(0.0, 1.0, 0.0, 1.0);
    const pointZ1 = math.Vec4f.from(0.0, 0.0, -1.0, 1.0);
    const pointZ2 = math.Vec4f.from(0.0, 0.0, 1.0, 1.0);

    const pX1 = viewport.apply_to_vec4(mat.apply_to_vec4(pointX1).perspective_div());
    const pX2 = viewport.apply_to_vec4(mat.apply_to_vec4(pointX2).perspective_div());
    const pY1 = viewport.apply_to_vec4(mat.apply_to_vec4(pointY1).perspective_div());
    const pY2 = viewport.apply_to_vec4(mat.apply_to_vec4(pointY2).perspective_div());
    const pZ1 = viewport.apply_to_vec4(mat.apply_to_vec4(pointZ1).perspective_div());
    const pZ2 = viewport.apply_to_vec4(mat.apply_to_vec4(pointZ2).perspective_div());

    // const pX1 = mat.apply_to_vec4(pointX1).perspective_div();
    // const pX2 = mat.apply_to_vec4(pointX2).perspective_div();
    // const pY1 = mat.apply_to_vec4(pointY1).perspective_div();
    // const pY2 = mat.apply_to_vec4(pointY2).perspective_div();
    // const pZ1 = mat.apply_to_vec4(pointZ1).perspective_div();
    // const pZ2 = mat.apply_to_vec4(pointZ2).perspective_div();

    line(db.*, math.Vec2f.from(pX1.x, pX1.y), math.Vec2f.from(pX2.x, pX2.y), pixel.BGRA.make(0x00, 0x00, 0xff, 0x00));
    line(db.*, math.Vec2f.from(pY1.x, pY1.y), math.Vec2f.from(pY2.x, pY2.y), pixel.BGRA.make(0x00, 0xff, 0x00, 0x00));
    line(db.*, math.Vec2f.from(pZ1.x, pZ1.y), math.Vec2f.from(pZ2.x, pZ2.y), pixel.BGRA.make(0xff, 0x00, 0x00, 0x00));
    _ = x11.XPutImage(db.*.display, db.*.window, db.*.gc, db.*.image, wa.x, wa.y, wa.x, wa.y, @intCast(wa.width), @intCast(wa.height));

    _ = x11.XFlush(db.*.display);
}

pub fn processEvents(db: *DB) void {
    var wm_delete_window = x11.XInternAtom(db.*.display, "WM_DELETE_WINDOW", x11.False);
    if (wm_delete_window == x11.None) {
        std.debug.print("Failed to intern WM_DELETE_WINDOW atom\n", .{});
        return;
    }
    const atoms: [*c]x11.Atom = &wm_delete_window;

    _ = x11.XSetWMProtocols(db.*.display, db.*.window, atoms, 1);

    var prev_x: i32, var prev_y: i32, var button_pressed: bool = .{ 0, 0, false };

    var event: x11.XEvent = undefined;
    var running = true;
    while (running) {
        _ = x11.XNextEvent(db.*.display, &event);

        switch (event.type) {
            x11.ClientMessage => {
                if (event.xclient.data.l[0] == wm_delete_window) {
                    running = false;
                    break;
                }
            },
            x11.Expose => {
                redraw(db);
            },
            x11.KeyPress => {},
            x11.KeyRelease => {
                if (x11.XLookupKeysym(&event.xkey, 0) == x11.XK_q) {
                    running = false;
                }
            },
            x11.MotionNotify => {
                if (button_pressed) {
                    const x, const y = .{ event.xmotion.x, event.xmotion.y };

                    db.*.camera.yaw += math.radians(@floatFromInt(x - prev_x)) * 0.1;

                    db.*.camera.pitch += math.radians(@floatFromInt(y - prev_y)) * 0.1;

                    if (db.*.camera.pitch > std.math.pi / 2.0 - 0.1) {
                        db.*.camera.pitch = std.math.pi / 2.0 - 0.1;
                    }

                    if (db.*.camera.pitch < -std.math.pi / 2.0 + 0.1) {
                        db.*.camera.pitch = -std.math.pi / 2.0 + 0.1;
                    }

                    prev_x, prev_y = .{ x, y };

                    redraw(db);

                    _ = x11.XSync(db.*.display, x11.True);
                }
            },
            x11.ButtonPress => {
                switch (event.xbutton.button) {
                    x11.Button1 => {
                        button_pressed = true;
                        prev_x, prev_y = .{ event.xbutton.x, event.xbutton.y };
                    },
                    x11.Button4 => {
                        db.*.camera.radius += 0.5;
                        if (db.*.camera.radius > 100.0) {
                            db.*.camera.radius = 100.0;
                        }
                    },
                    x11.Button5 => {
                        db.*.camera.radius -= 0.5;
                        if (db.*.camera.radius < 1.0) {
                            db.*.camera.radius = 1.0;
                        }
                    },
                    else => {},
                }

                redraw(db);

                _ = x11.XSync(db.*.display, x11.True);
            },
            x11.ButtonRelease => {
                if (event.xbutton.button == x11.Button1)
                    button_pressed = false;
            },
            else => {},
        }
    }
}

pub fn main() !void {
    const mesh = model.parse_file(std.heap.page_allocator, "./objects/model.obj") catch |err| {
        return err;
    };

    const display = x11.XOpenDisplay(null) orelse {
        return Error.FailedOpenDisplay;
    };
    defer _ = x11.XCloseDisplay(display);

    const screen = x11.DefaultScreen(display);
    const visual = x11.DefaultVisual(display, screen);

    const window = try createWindow(display, screen, visual);
    defer {
        _ = x11.XDestroyWindow(display, window);
        _ = x11.XFreeColormap(display, x11.XDefaultColormap(display, screen));
    }

    _ = x11.XMapWindow(display, window);
    defer _ = x11.XUnmapWindow(display, window);

    var wa: x11.XWindowAttributes = undefined;
    if (x11.XGetWindowAttributes(display, window, &wa) == 0) {
        return Error.FailedFindRootWindow;
    }

    const gc = x11.XCreateGC(display, window, 0, null) orelse {
        return Error.FailedCreateGC;
    };
    defer _ = x11.XFreeGC(display, gc);

    var gpa = std.heap.GeneralPurposeAllocator(.{}){};
    defer _ = gpa.deinit();
    const allocator = gpa.allocator();

    const buffer = allocator.alloc(pixel.BGRA, 1920 * 1080) catch |err| {
        return err;
    };
    defer _ = allocator.free(buffer);

    const camera = SphereCamera.from(math.Vec3f.from(0.0, 0.0, 0.0));
    const vertices = allocator.alloc(math.Vec4f, mesh.vertices.len) catch |err| {
        std.log.err("allocation error {}", .{err});
        return;
    };
    defer _ = allocator.free(vertices);

    var db: DB = .{
        .camera = camera,
        .vertices = vertices,
        .allocator = allocator,
        .mesh = mesh,
        .display = display,
        .window = window,
        .pixels = Buffer2D(pixel.BGRA).from(buffer, 1920),
        .gc = gc,
        .image = x11.XCreateImage(
            display,
            wa.visual,
            @intCast(wa.depth),
            x11.ZPixmap,
            0,
            @ptrCast(buffer.ptr),
            1920,
            1080,
            32,
            1920 * @sizeOf(u32),
        ),
    };

    processEvents(&db);
}
