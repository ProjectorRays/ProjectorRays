var gulp = require("gulp");
var ts = require("gulp-typescript");

var tsProject = ts.createProject("tsconfig.json");

gulp.task("default", function () {
  return gulp.src("src/**/*.ts")
    .pipe(tsProject())
    .pipe(gulp.dest("bin"));
});
